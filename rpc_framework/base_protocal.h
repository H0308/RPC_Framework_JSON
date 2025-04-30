#ifndef __rpc_base_protocal_h__
#define __rpc_base_protocal_h__

#include <memory>
#include <rpc_framework/base_buffer.h>
#include <rpc_framework/base_message.h>
#include <rpc_framework/public_data.h>

namespace base_protocal
{
    // 抽象协议类
    class BaseProtocol
    {
    public:
        using ptr = std::shared_ptr<BaseProtocol>;
        // 判断是否是有效数据
        virtual bool canProcessed(const base_buffer::BaseBuffer::ptr &buf) = 0;
        // 收到消息时的处理，从buffer中读取数据交给Message类处理
        virtual bool getContentFromProtocal(const base_buffer::BaseBuffer::ptr &buf, base_message::BaseMessage::ptr &msg) = 0;
        // 序列化接口，用于序列化Message类的成员
        virtual std::string constructProtocal(const base_message::BaseMessage::ptr &msg) = 0;
        // 不提供反序列化
    };
}

#endif