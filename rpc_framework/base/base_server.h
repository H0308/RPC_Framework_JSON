#ifndef __rpc_base_server_h__
#define __rpc_base_server_h__

#include <memory>
#include <rpc_framework/base/public_data.h>

namespace base_server
{
    // 服务端抽象
    class BaseServer
    {
    public:
        using ptr = std::shared_ptr<BaseServer>;
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

        // 启动服务器
        virtual void start() = 0;

    protected:
        public_data::connectionCallback_t cb_connection_;
        public_data::closeCallback_t cb_close_;
        public_data::messageCallback_t cb_message_;
    };
}


#endif