#ifndef __rpc__server_factory_h__
#define __rpc__server_factory_h__

#include <rpc_framework/base_server.h>
#include <rpc_framework/muduo_server.h>

namespace server_factory
{
    class ServerFactory
    {
    public:
        template <class... Args>
        static base_server::BaseServer::ptr serverCreateFactory(Args &&...args)
        {
            return std::make_shared<muduo_server::MuduoServer>(std::forward<Args>(args)...);
        }
    };
}

#endif