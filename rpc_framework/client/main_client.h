#ifndef __rpc_main_client_h__
#define __rpc_main_client_h__

#include <rpc_framework/client/requestor.h>
#include <rpc_framework/client/rpc_registry_client.h>
#include <rpc_framework/client/rpc_caller.h>
#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/base/base_client.h>
#include <rpc_framework/factories/client_factory.h>

namespace rpc_client
{
    namespace main_client
    {
        // 用于服务注册的客户端
        class RegisterClient
        {
        public:
            using ptr = std::shared_ptr<RegisterClient>;
            RegisterClient(const std::string &ip, const uint16_t port)
                : requestor_(std::make_shared<requestor_rpc_framework::Requestor>()), provider_(std::make_shared<rpc_client::rpc_registry::Provider>(requestor_)), dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>())
            {
                dispatcher_->registerService<base_message::BaseMessage>(public_data::MType::Resp_service, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor_.get(), std::placeholders::_1, std::placeholders::_2));

                client_ = client_factory::ClientFactory::clientCreateFactory(ip, port);
                client_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 连接服务端
                client_->connect();
            }

            bool toRegisterService(const std::string &method, const public_data::host_addr_t &host)
            {
                return provider_->registerService(client_->connection(), method, host);
            }

        private:
            // requestor需要在provider之前定义，因为provider依赖requestor
            requestor_rpc_framework::Requestor::ptr requestor_; 
            rpc_client::rpc_registry::Provider::ptr provider_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            base_client::BaseClient::ptr client_;
        };

        // 用于服务发现的客户端
        class DiscovererClient
        {
        public:
            using ptr = std::shared_ptr<DiscovererClient>;
            DiscovererClient(const std::string &ip, const uint16_t port)
                : requestor_(std::make_shared<requestor_rpc_framework::Requestor>()), discoverer_(std::make_shared<rpc_client::rpc_registry::Discoverer>(requestor_)), dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>())
            {
                // 处理服务发现请求对应的响应
                dispatcher_->registerService<base_message::BaseMessage>(public_data::MType::Resp_service, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor_.get(), std::placeholders::_1, std::placeholders::_2));

                // 处理服务上线/下线请求对应的响应
                dispatcher_->registerService<request_message::ServiceRequest>(public_data::MType::Resp_service, std::bind(&rpc_client::rpc_registry::Discoverer::handleOnlineOfflineServiceRequest, discoverer_.get(),std::placeholders::_1, std::placeholders::_2));

                client_ = client_factory::ClientFactory::clientCreateFactory(ip, port);
                client_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 连接服务端
                client_->connect();
            }
            bool toHandleDiscoveryRequest(const std::string &method, public_data::host_addr_t &host)
            {
                return discoverer_->handleDiscoveryRequest(client_->connection(), method, host);
            }

        private:
            // requestor在discoverer之前
            requestor_rpc_framework::Requestor::ptr requestor_;
            rpc_client::rpc_registry::Discoverer::ptr discoverer_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
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