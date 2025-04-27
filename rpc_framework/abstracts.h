#ifndef __rpc_abstracts_h__
#define __rpc_abstracts_h__

#include <memory>
#include <functional>
#include "public_data.h"
#include "base_message.h"

namespace abstracts
{
    using namespace public_data;
    using namespace base_message;

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

    // 抽象协议类
    class BaseProtocol
    {
    public:
        using ptr = std::shared_ptr<BaseProtocol>;
        // 判断是否是有效数据
        virtual bool canProcessed(const BaseBuffer::ptr &buf) = 0;
        // 收到消息时的处理，从buffer中读取数据交给Message类处理
        virtual bool onMessage(const BaseBuffer::ptr &buf, BaseMessage::ptr &msg) = 0;
        // 序列化接口，用于序列化Message类的成员
        virtual std::string serialize(const BaseMessage::ptr &msg) = 0;
        // 不提供反序列化
    };

    // 抽象连接类
    class BaseConnection
    {
    public:
        using ptr = std::shared_ptr<BaseConnection>;
        // 发送
        virtual void send(const BaseMessage::ptr &msg) = 0;
        // 关闭连接
        virtual void shutdown() = 0;
        // 判断连接是否正常
        virtual bool connected() = 0;
    };

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