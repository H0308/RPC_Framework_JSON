#ifndef __rpc_base_server_h__
#define __rpc_base_server_h__

#include <memory>
#include <rpc_framework/public_data.h>

namespace base_server
{
    // 服务端抽象
    class BaseServer
    {
    public:
        using ptr = std::shared_ptr<BaseServer>;
        virtual void setConnectionCallback(const public_data::connectionCallback_t &cb)
        {
            _cb_connection = cb;
        }
        virtual void setCloseCallback(const public_data::closeCallback_t &cb)
        {
            _cb_close = cb;
        }
        virtual void setMessageCallback(const public_data::messageCallback_t &cb)
        {
            _cb_message = cb;
        }

        // 启动服务器
        virtual void start() = 0;

    protected:
        public_data::connectionCallback_t _cb_connection;
        public_data::closeCallback_t _cb_close;
        public_data::messageCallback_t _cb_message;
    };
}


#endif