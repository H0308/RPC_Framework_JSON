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
        template<class ...Args>
        static base_buffer::BaseBuffer::ptr bufferCreateFactory(Args&&... args)
        {
            return std::make_shared<muduo_buffer::MuduoBuffer>(std::forward<Args>(args)...);
        }
    };
}

#endif