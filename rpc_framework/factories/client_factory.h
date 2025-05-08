#ifndef __rpc_client_factory_h__
#define __rpc_client_factory_h__

#include <rpc_framework/base/base_client.h>
#include <rpc_framework/base/muduo_client.h>

namespace client_factory
{
    class ClientFactory
    {
    public:
        template <class... Args>
        static base_client::BaseClient::ptr clientCreateFactory(Args &&...args)
        {
            return std::make_shared<muduo_client::MuduoClient>(std::forward<Args>(args)...);
        }
    };
}

#endif