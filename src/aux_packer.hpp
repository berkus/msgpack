// C++11 variadic template version of the msgpack unpacker defines.
// From https://github.com/msgpack/msgpack-c/issues/11
//
namespace msgpack {

template <typename Packer>
struct aux_packer
{
    explicit aux_packer(Packer& pk)
    : m_pk(pk)
    {
    }

    template< typename ... args_t >
    void operator()(const args_t& ... args)
    {
        m_pk.pack_array(sizeof...(args_t));
        auto s = {
            (m_pk.pack(args), 1)...
        };
        (void)s;
    }

    void operator()()
    {
        m_pk.pack_array(1);
    }

private:
    aux_packer(const aux_packer & rhs) = delete;
    aux_packer & operator = (const aux_packer & rhs) = delete;
private:
    Packer & m_pk;
};

class aux_unpacker1
{
public:
    explicit aux_unpacker1(msgpack::object & obj)
        : m_o(obj)
    {
    }

    template< typename ... args_t >
    void operator()(args_t& ... args)
    {
        if(m_o.type != type::ARRAY) { throw type_error(); }

        if (m_o.via.array.size != sizeof...(args_t)) {
            throw type_error();
        }

        size_t idx = 0;
        auto s = {
            (m_o.via.array.ptr[idx++].convert(&args), 1)...
        };

        (void)s;
    }

    void operator()()
    {
        if(m_o.type != type::ARRAY) { throw type_error(); }
    }
private:
    aux_unpacker1(const aux_unpacker1 & rhs) = delete;
    aux_unpacker1 & operator = (const aux_unpacker1 & rhs) = delete;
private:
    msgpack::object & m_o;
};

template 
struct aux_unpacker2
{
    explicit aux_unpacker2(MSGPACK_OBJECT & o, msgpack::zone & z)
        : m_o(o)
        , m_z(z)
    {
    }

    template< typename ... args_t >
    void operator()(args_t& ... args)
    {
        m_o.type = type::ARRAY;

        m_o.via.array.ptr = (object*)m_z.malloc(
            sizeof(object) * sizeof...(args_t));
        m_o.via.array.size = sizeof...(args_t);

        size_t idx = 0;
        auto s = {
            (m_o.via.array.ptr[idx++] = object(args, m_z), 1)...
        };

        (void)s;
    }

    void operator()()
    {
        m_o.type = type::ARRAY;
        m_o.via.array.ptr = NULL;
        m_o.via.array.size = 0;
    }

private:
    aux_unpacker2(const aux_unpacker2 & rhs) = delete;
    aux_unpacker2 & operator = (const aux_unpacker2 & rhs) = delete;
private:
    MSGPACK_OBJECT & m_o;
    msgpack::zone & m_z;
};

} /* msgpack */

#define MSG_PACKER_DEFINE(...) \
    template \
    void msgpack_pack(Packer& pk) const \
    { \
        msgpack::aux_packer tmp(pk); \
        tmp(__VA_ARGS__); \
    } \
    void msgpack_unpack(msgpack::object o) \
    { \
        msgpack::aux_unpacker1 tmp(o); \
        tmp(__VA_ARGS__); \
    }\
    template \
    void msgpack_object(MSGPACK_OBJECT* o, msgpack::zone* z) const \
    { \
        msgpack::aux_unpacker2 tmp(*o, *z); \
        tmp(__VA_ARGS__); \
    }
