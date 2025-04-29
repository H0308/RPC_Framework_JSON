#ifndef __rpc_uuid_generator_h__
#define __rpc_uuid_generator_h__

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace uuid_generator
{
    class UuidGenerator
    {
    public:
        static std::string generate_uuid()
        {
            // 使用boost库生成uuid
            // 创建一个随机数生成器
            boost::uuids::random_generator generator;

            // 生成一个随机 UUID
            boost::uuids::uuid id = generator();
            return boost::uuids::to_string(id);
        }
    };
}

#endif