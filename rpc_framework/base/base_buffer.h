#ifndef __rpc_base_buffer_h__
#define __rpc_base_buffer_h__

#include <memory>
#include <rpc_framework/base/public_data.h>

namespace base_buffer
{
    // 抽象缓冲区
    class BaseBuffer
    {
    public:
        using ptr = std::shared_ptr<BaseBuffer>;
        // 可读数据大小
        virtual size_t readableSize() = 0;
        // 尝试获取4字节数据，但是不从缓冲区删除
        virtual int32_t peekInt32() = 0;
        // 删除4字节数据
        virtual void retrieveInt32() = 0;
        // 读取并删除4字节数据
        virtual int32_t readInt32() = 0;
        // 获取指定长度的数据
        virtual std::string retrieveAsString(size_t len) = 0;
    };
}

#endif