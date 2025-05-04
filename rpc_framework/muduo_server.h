#ifndef __rpc_muduo_server_h__
#define __rpc_muduo_server_h__

#include <rpc_framework/base_server.h>
#include <muduo_include/muduo/net/EventLoop.h>
#include <muduo_include/muduo/net/TcpServer.h>

namespace muduo_server
{
    // 基于Muduo库中的TcpServer实现
    class MuduoServer : public base_server::BaseServer
    {
    public:
        MuduoServer(uint16_t port)
            : server_(loop_.get(),
                      muduo::net::InetAddress("0.0.0.0", port),
                      "dict_server",
                      muduo::net::TcpServer::kReusePort),
              loop_(std::make_shared<muduo::net::EventLoop>())
        {
        }

        // 启动服务器
        virtual void start() override
        {
            // 设置回调
            // 1. 连接回调
            server_.setConnectionCallback([this](const muduo::net::TcpConnectionPtr &con)
                                          { this->connectionCallback(con); });
            // 2. 消息回调
            server_.setMessageCallback([this](const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp t)
                                       { this->messageCallback(con, buffer, t); });

            server_.start();
            loop_->loop();
        }

    private:
        void connectionCallback(const muduo::net::TcpConnectionPtr &con)
        {
            // 本次实现：连接成功和断开连接进行提示
            if (con->connected())
            {
                
            }
            else if (con->disconnected())
            {

            }
        }

        void messageCallback(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp t)
        {
            
        }

    private:
        std::shared_ptr<muduo::net::EventLoop> loop_; // 事件模型，先初始化
        muduo::net::TcpServer server_;                // 服务器
        std::unordered_map<muduo::net::TcpConnectionPtr, base_connection::BaseConnection::ptr> tcp_cons; // Muduo链接和封装连接进行映射，用于管理连接结构

    };
}

#endif