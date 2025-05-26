#ifndef __rpc_base_client_h__
#define __rpc_base_client_h__

#include <memory>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/public_data.h>

namespace base_client
{
    // 客户端抽象
    class BaseClient
    {
    public:
        using ptr = std::shared_ptr<BaseClient>;
        virtual void setConnectionCallback(const public_data::connectionCallback_t &cb)
        {
            cb_connection_ = cb;
        }
        virtual void setCloseCallback(const public_data::closeCallback_t &cb)
        {
            cb_close_ = cb;
        }
        virtual void setMessageCallback(const public_data::messageCallback_t &cb)
        {
            cb_message_ = cb;
        }

        // 连接服务端
        virtual void connect() = 0;
        // 关闭连接
        virtual void shutdown() = 0;
        // 获取连接对象
        virtual base_connection::BaseConnection::ptr connection() = 0;
        // 判断是否连接
        virtual bool connected() = 0;

    protected:
        public_data::connectionCallback_t cb_connection_;
        public_data::closeCallback_t cb_close_;
        public_data::messageCallback_t cb_message_;
    };
}

#endif
