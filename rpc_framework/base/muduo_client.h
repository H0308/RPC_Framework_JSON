#ifndef __rpc_muduo_client_h__
#define __rpc_muduo_client_h__

#include <rpc_framework/base/base_client.h>
#include <rpc_framework/base/log.h>
#include <rpc_framework/factories/connection_factory.h>
#include <rpc_framework/factories/protocol_factory.h>
#include <rpc_framework/factories/buffer_factory.h>
#include <rpc_framework/muduo_include/muduo/net/TcpClient.h>
#include <rpc_framework/muduo_include/muduo/net/EventLoopThread.h>
#include <rpc_framework/muduo_include/muduo/net/EventLoop.h>
#include <rpc_framework/muduo_include/muduo/base/CountDownLatch.h>

namespace muduo_client
{
    using namespace log_system;
    class MuduoClient : public base_client::BaseClient
    {
    public:
        using ptr = std::shared_ptr<MuduoClient>;
        MuduoClient(const std::string &ip, uint16_t port)
            : loop_(loopThread_.startLoop()), client_(loop_, muduo::net::InetAddress(ip, port), "MuduoClient"),
              pro_(protocol_factory::ProtocolFactory::createProtocolFactory()),
              count_(1) // 确保客户端在连接建立成功后发送消息
        {
            // 设置回调函数
            // 1. 连接回调
            client_.setConnectionCallback(std::bind(&MuduoClient::connectionCallback, this, std::placeholders::_1));
            // 2. 消息回调
            client_.setMessageCallback(std::bind(&MuduoClient::messgaeCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        }

        // 连接服务端
        virtual void connect() override
        {
            client_.connect();
            // 客户端开始在同步计数器等待，防止未连接时发送信息
            count_.wait();
        }

        // 关闭连接
        virtual void shutdown() override
        {
            // 调用TcpClient的断开接口
            client_.disconnect();
        }

        // 发送消息
        virtual bool send(const base_message::BaseMessage::ptr &msg) override
        {
            // 调用BaseConnection中的发送接口
            if(!con_)
            {
                LOG(Level::Error, "客户端发送失败，连接不存在");
                return false;
            }

            con_->send(msg);

            return true;
        }

        // 获取连接对象
        virtual base_connection::BaseConnection::ptr connection() override
        {
            return con_;
        }

        // 判断是否连接
        virtual bool connected() override
        {
            return con_ && con_->connected();
        }

    private:
        // 连接回调函数
        // 当连接成功时，创建当前客户端的BaseConnection对象
        // 当连接失败时，重置连接
        void connectionCallback(const muduo::net::TcpConnectionPtr &con)
        {
            if (con->connected())
            {
                LOG(Level::Warning, "客户端连接成功");
                // 设置连接对象指针，便于接下来调用send
                con_ = connection_factory::ConnectionFactory::createConnectionFactory(pro_, con);
                // 更改同步计数器，减到0表示成功连接，唤醒客户端，可以进行消息发送
                count_.countDown();
            }
            else if (con->disconnected())
            {
                std::cout << "客户端断开连接" << std::endl;
                // 重置连接指针
                con_.reset();
            }
        }

        // 收到消息时的回调
        void messgaeCallback(const muduo::net::TcpConnectionPtr &con, muduo::net::Buffer *buffer, muduo::Timestamp t)
        {
            // 创建出BaseBuffer对象
            base_buffer::BaseBuffer::ptr b_buffer = buffer_factory::BufferFactory::bufferCreateFactory(buffer);

            // 判断缓冲区中的数据是否可以处理（数据不会过小，也不会过大）
            while (true)
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
                if (!pro_->getContentFromProtocol(b_buffer, b_msg))
                {
                    LOG(Level::Warning, "反序列化处理失败");
                    break;
                }

                // 如果设置了回调函数就处理
                // 处理收到的消息
                if (cb_message_)
                    cb_message_(con_, b_msg);
            }
        }

    private:
        muduo::net::EventLoopThread loopThread_;
        muduo::net::EventLoop* loop_; // 不能使用智能指针管理EventLoop对象
        muduo::net::TcpClient client_;
        base_connection::BaseConnection::ptr con_;
        muduo::CountDownLatch count_;
        base_protocol::BaseProtocol::ptr pro_;
    };
}

#endif