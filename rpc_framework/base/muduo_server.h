#ifndef __rpc_muduo_server_h__
#define __rpc_muduo_server_h__

#include <mutex>
#include <rpc_framework/base/base_server.h>
#include <rpc_framework/factories/connection_factory.h>
#include <rpc_framework/factories/protocol_factory.h>
#include <rpc_framework/factories/buffer_factory.h>
#include <rpc_framework/muduo_include/muduo/net/EventLoop.h>
#include <rpc_framework/muduo_include/muduo/net/TcpServer.h>

namespace muduo_server
{
    using namespace log_system;
    // 基于Muduo库中的TcpServer实现
    class MuduoServer : public base_server::BaseServer
    {
    public:
        using ptr = std::shared_ptr<MuduoServer>;

        MuduoServer(uint16_t port)
            : server_(loop_.get(),
                      muduo::net::InetAddress("0.0.0.0", port),
                      "dict_server",
                      muduo::net::TcpServer::kReusePort),
              loop_(std::make_shared<muduo::net::EventLoop>()),
              pro_(protocol_factory::ProtocolFactory::createProtocolFactory())
        {
            // 设置回调
            // 1. 连接回调
            server_.setConnectionCallback([this](const muduo::net::TcpConnectionPtr &con)
                                          { this->connectionCallback(con); });
            // 2. 消息回调
            server_.setMessageCallback([this](const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp t)
                                       { this->messageCallback(con, buffer, t); });
        }

        // 启动服务器
        virtual void start() override
        {
            server_.start();
            loop_->loop();
        }

    private:
        // 连接回调函数，用于管理TcpConnection和BaseConnection
        // 连接建立成功，则创建BaseConnection对象并将对应地{TcpConnection, BaseConnection}插入到哈希表中，再根据是否设置连接回调函数选择是否执行该函数
        // 连接断开时，需要将BaseConnection对象从哈希表中移除，并根据是否设置连接关闭回调函数选择是否执行该函数
        void connectionCallback(const muduo::net::TcpConnectionPtr &con)
        {
            if (con->connected())
            {
                // 创建BaseConnection对象
                base_connection::BaseConnection::ptr b_con = connection_factory::ConnectionFactory::createConnectionFactory(pro_, con);
                {
                    // 插入到哈希表
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    tcp_cons_.insert({con, b_con});
                }

                // 如果设置了回调就调用
                // 处理连接
                if(cb_connection_) 
                    cb_connection_(b_con);
            }
            else if (con->disconnected())
            {
                // 查找是否存在对应的BaseConnection对象
                base_connection::BaseConnection::ptr b_con;
                {
                    auto pos = tcp_cons_.find(con);
                    if(pos == tcp_cons_.end())
                    {
                        LOG(Level::Warning, "不存在指定的连接");
                        con->shutdown();
                        return ;
                    }
                    // 找到了就获取
                    b_con = pos->second;
                    // 移除键值对
                    tcp_cons_.erase(con);
                }

                // 如果设置了回调就调用
                // 关闭BaseConnection
                if(cb_close_)
                    cb_close_(b_con);
            }
        }

        // 客户端发送消息时的处理
        void messageCallback(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp t)
        {
            // 创建出BaseBuffer对象
            base_buffer::BaseBuffer::ptr b_buffer = buffer_factory::BufferFactory::bufferCreateFactory(buffer);

            // 判断缓冲区中的数据是否可以处理（数据不会过小，也不会过大）
            while(true)
            {
                if (!pro_->canProcessed(b_buffer))
                {
                    // 无法处理时也有可能是数据过大
                    if (b_buffer->readableSize() >= public_data::max_data_size)
                    {
                        LOG(Level::Warning, "数据过大，无法处理");
                        break;
                    }

                    // 否则就是数据过小（无法满足LV协议格式的处理规则）
                    break;
                }

                // 创建BaseMessage对象指针，交给BaseProtocol中的反序列化接口创建对象
                base_message::BaseMessage::ptr b_msg;
                if(!pro_->getContentFromBuffer(b_buffer, b_msg))
                {
                    LOG(Level::Warning, "反序列化处理失败");
                    break;
                }

                // 如果设置了回调函数就处理
                // 处理收到的消息
                base_connection::BaseConnection::ptr b_con;
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    auto pos = tcp_cons_.find(con);
                    // 不存在连接时直接断开，防止之后的处理也出现异常
                    if(pos == tcp_cons_.end())
                    {
                        LOG(Level::Warning, "不存在指定的连接");
                        con->shutdown();
                        break;
                    }

                    b_con = pos->second;
                }

                if(cb_message_)
                    cb_message_(b_con, b_msg);
            }

        }

    private:
        std::shared_ptr<muduo::net::EventLoop> loop_; // 事件模型，先初始化
        muduo::net::TcpServer server_;                // 服务器
        std::unordered_map<muduo::net::TcpConnectionPtr, base_connection::BaseConnection::ptr> tcp_cons_; // Muduo链接和封装连接进行映射，用于管理连接结构
        std::mutex manage_map_mtx_; // 管理哈希表保证线程安全
        base_protocol::BaseProtocol::ptr pro_; // 创建MuduoConnection时需要
    };
}

#endif