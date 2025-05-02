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

    // 连接回调函数
    using connectionCallback_t = std::function<void(const BaseConnection::ptr &)>;
    // 关闭连接时回调函数
    using closeCallback_t = std::function<void(const BaseConnection::ptr &)>;
    // 收到消息时回调函数
    using messageCallback_t = std::function<void(const BaseConnection::ptr &, BaseMessage::ptr &)>;

    // 服务端抽象
    class BaseServer
    {
    public:
        using ptr = std::shared_ptr<BaseServer>;
        virtual void setConnectionCallback(const connectionCallback_t &cb)
        {
            _cb_connection = cb;
        }
        virtual void setCloseCallback(const closeCallback_t &cb)
        {
            _cb_close = cb;
        }
        virtual void setMessageCallback(const messageCallback_t &cb)
        {
            _cb_message = cb;
        }

        // 启动服务器
        virtual void start() = 0;

    protected:
        connectionCallback_t _cb_connection;
        closeCallback_t _cb_close;
        messageCallback_t _cb_message;
    };

    // 客户端抽象
    class BaseClient
    {
    public:
        using ptr = std::shared_ptr<BaseClient>;
        virtual void setConnectionCallback(const connectionCallback_t &cb)
        {
            _cb_connection = cb;
        }
        virtual void setCloseCallback(const closeCallback_t &cb)
        {
            _cb_close = cb;
        }
        virtual void setMessageCallback(const messageCallback_t &cb)
        {
            _cb_message = cb;
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
        connectionCallback_t _cb_connection;
        closeCallback_t _cb_close;
        messageCallback_t _cb_message;
    };
}

#endif