#ifndef __rpc_main_client_h__
#define __rpc_main_client_h__

#include <rpc_framework/client/requestor.h>
#include <rpc_framework/client/rpc_registry_client.h>
#include <rpc_framework/client/rpc_caller.h>
#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/base/base_client.h>

namespace rpc_client
{
    namespace main_client
    {
        // 用于服务注册的客户端
        class RegisterClient
        {
        public:
            using ptr = std::shared_ptr<RegisterClient>;
            bool toRegisterService(const std::string &method, const public_data::host_addr_t &host)
            {
            }

        private:
            rpc_client::rpc_registry::Provider::ptr provider_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            requestor_rpc_framework::Requestor::ptr requestor_;
            base_client::BaseClient::ptr client_;
        };

        // 用于服务发现的客户端
        class DiscovererClient
        {
        public:
            using ptr = std::shared_ptr<DiscovererClient>;
            bool handleDiscoveryRequest(const std::string &method, public_data::host_addr_t &host)
            {
            }

        private:
            rpc_client::rpc_registry::Discoverer::ptr provider_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            requestor_rpc_framework::Requestor::ptr requestor_;
            base_client::BaseClient::ptr client_;
        };

        // 用于RPC调用的客户端
        class RpcClient
        {
        public:
            using ptr = std::shared_ptr<RpcClient>;
            // 同步函数
            bool call(const std::string &method_name, const Json::Value &params, Json::Value &result)
            {
            }

            // 异步函数
            bool call(const std::string &method_name, const Json::Value &params, rpc_client::rpc_caller::RpcCaller::aysnc_response &result)
            {
            }

            // 回调函数
            bool call(const std::string &method_name, const Json::Value &params, const rpc_client::rpc_caller::RpcCaller::callback_t &cb)
            {
            }

        private:
            DiscovererClient::ptr discoverer_client;
            rpc_client::rpc_caller::RpcCaller::ptr rpc_caller_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            requestor_rpc_framework::Requestor::ptr requestor_;
            base_client::BaseClient::ptr client_;
        };
    }
}

#endif