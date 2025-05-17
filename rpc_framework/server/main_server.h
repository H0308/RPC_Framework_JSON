#ifndef __rpc_main_server_h__
#define __rpc_main_server_h__

#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/server/rpc_registry_server.h>
#include <rpc_framework/factories/server_factory.h>

namespace rpc_server
{
    namespace main_server
    {
        class RegistryServer
        {
        public:
            RegistryServer(uint16_t port)
                : pd_manager_(std::make_shared<rpc_registry::ProviderDiscovererManager>()),
                dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>())
            {
                // 向dispatcher模块中绑定回调函数
                dispatcher_->registerService<request_message::ServiceRequest>(public_data::MType::Req_service,std::bind(&rpc_registry::ProviderDiscovererManager::handleRegisterDiscoverRequest, pd_manager_.get(), std::placeholders::_1, std::placeholders::_2));

                // 创建服务器对象并添加消息回调
                server_ = server_factory::ServerFactory::serverCreateFactory(port);
                server_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 绑定连接断开回调
                server_->setCloseCallback(std::bind(&RegistryServer::handleConnectionCallback, this, std::placeholders::_1));
            }

            void start()
            {
                server_->start();
            }

        private:
            // 提供连接断开回调的封装函数
            void handleConnectionCallback(const base_connection::BaseConnection::ptr &con)
            {
                pd_manager_->handleProviderConnectionShutdown(con);
            }

        private:
            rpc_registry::ProviderDiscovererManager::ptr pd_manager_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            base_server::BaseServer::ptr server_;
        };
    }
}

#endif