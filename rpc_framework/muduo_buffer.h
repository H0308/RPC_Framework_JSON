#ifndef __rpc_muduo_buffer_h__
#define __rpc_muduo_buffer_h__

#include <rpc_framework/base_buffer.h>
#include <rpc_framework/muduo_include/muduo/net/Buffer.h>

namespace muduo_buffer
{
    // 基于Muduo库中的Buffer进行再次封装，满足可扩展性
    // 方法实现底层全部调用Muduo中Buffer类中的方法
    class MuduoBuffer : public base_buffer::BaseBuffer
    {
        const int valid_length_field_length = 4;

    public:
        MuduoBuffer(muduo::net::Buffer *buf)
            : buffer_(buf)
        {
        }
        using ptr = std::shared_ptr<MuduoBuffer>;
        // 可读数据大小
        virtual size_t readableSize() override
        {
            return buffer_->readableBytes();
        }
        // 尝试获取4字节数据，但是不从缓冲区删除
        virtual int32_t peekInt32() override
        {
            return buffer_->peekInt32(); // 会进行网络字节序转换
        }
        // 删除4字节数据
        virtual void retrieveInt32() override
        {
            buffer_->retrieveInt32();
        }
        // 读取并删除4字节数据
        virtual int32_t readInt32() override
        {
            return buffer_->readInt32();
        }
        // 获取指定长度的数据
        virtual std::string retrieveAsString(size_t len) override
        {
            return buffer_->retrieveAsString(len);
        }

    private:
        muduo::net::Buffer *buffer_; // 基于Muduo库的Buffer
    };
}

#endif