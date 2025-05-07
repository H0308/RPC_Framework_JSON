#ifndef __rpc_connection_factory_h__
#define __rpc_connection_factory_h__

#include <rpc_framework/base_connection.h>
#include <rpc_framework/muduo_connection.h>

namespace connection_factory
{
    class ConnectionFactory
    {
    public:
        template<class ...Args>
        static base_connection::BaseConnection::ptr createConnectionFactory(Args && ...args)
        {
            return std::make_shared<muduo_connection::MuduoConnection>(std::forward<Args>(args)...);
        }
    };
}

#endif