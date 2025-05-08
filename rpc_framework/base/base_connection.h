#ifndef __rpc_base_connection_h__
#define __rpc_base_connection_h__

#include <memory>
#include <rpc_framework/base/base_message.h>

namespace base_connection
{
    // 抽象连接类
    class BaseConnection
    {
    public:
        using ptr = std::shared_ptr<BaseConnection>;
        // 发送
        virtual void send(const base_message::BaseMessage::ptr &msg) = 0;
        // 关闭连接
        virtual void shutdown() = 0;
        // 判断连接是否正常
        virtual bool connected() = 0;
    };
}

#endif