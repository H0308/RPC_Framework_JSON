#ifndef __rpc__buffer_factory_h__
#define __rpc__buffer_factory_h__

#include <rpc_framework/base_buffer.h>
#include <rpc_framework/muduo_buffer.h>

namespace buffer_factory
{
    // 缓冲区工厂
    class BufferFactory
    {
    public:
        template<class T, class ...Args>
        static base_buffer::BaseBuffer bufferCreateFactory(Args... args)
        {
            return std::make_shared<T>(std::forward(args)...);
        }
    };
}

#endif