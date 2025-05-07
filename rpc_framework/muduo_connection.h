#ifndef __rpc_muduo_connection_h__
#define __rpc_muduo_connection_h__

#include <memory>
#include <rpc_framework/base_connection.h>
#include <rpc_framework/base_protocol.h>
#include <rpc_framework/muduo_include/muduo/net/TcpConnection.h>

namespace muduo_connection
{
    // 基于Muduo库的TcpConnection进行实现
    class MuduoConnection : public base_connection::BaseConnection
    {
    public:
        using ptr = std::shared_ptr<MuduoConnection>;

        MuduoConnection(const base_protocol::BaseProtocol::ptr &pro, const muduo::net::TcpConnectionPtr &con)
            : pro_(pro), con_(con)
        {
        }

        // 发送
        virtual void send(const base_message::BaseMessage::ptr &msg) override
        {
            // 获取待发送的数据
            std::string content = pro_->constructProtocol(msg);
            // 调用TcpConnection的发送
            con_->send(content);
        }
        // 关闭连接
        virtual void shutdown() override 
        {
            con_->shutdown();
        }
        // 判断连接是否正常
        virtual bool connected() override
        {
            return con_->connected();
        }

    private:
        base_protocol::BaseProtocol::ptr pro_; // 使用协议中的方法获取到待发送的数据
        muduo::net::TcpConnectionPtr con_;     // 使用Muduo库中的TcpConnection
    };
}

#endif