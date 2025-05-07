#ifndef __rpc_abstracts_h__
#define __rpc_abstracts_h__

#include <memory>
#include <functional>
#include "public_data.h"
#include "base_message.h"
#include "base_buffer.h"
#include "base_connection.h"

namespace abstracts
{
    using namespace public_data;
    using namespace base_message;
    using namespace base_buffer;
    using namespace base_connection;

    

    

    // 客户端抽象
    class BaseClient
    {
    public:
        using ptr = std::shared_ptr<BaseClient>;
        virtual void setConnectionCallback(const connectionCallback_t &cb)
        {
            cb_connection_ = cb;
        }
        virtual void setCloseCallback(const closeCallback_t &cb)
        {
            cb_close_ = cb;
        }
        virtual void setMessageCallback(const messageCallback_t &cb)
        {
            cb_message_ = cb;
        }

        // 连接服务端
        virtual void connect() = 0;
        // 关闭连接
        virtual void shutdown() = 0;
        // 发送消息
        virtual bool send(const BaseMessage::ptr &) = 0;
        // 获取连接对象
        virtual BaseConnection::ptr connection() = 0;
        // 判断是否连接
        virtual bool connected() = 0;

    protected:
        connectionCallback_t cb_connection_;
        closeCallback_t cb_close_;
        messageCallback_t cb_message_;
    };
}

#endif