#ifndef __rpc_main_server_h__
#define __rpc_main_server_h__

#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/server/rpc_registry_server.h>
#include <rpc_framework/factories/server_factory.h>
#include <rpc_framework/client/main_client.h>
#include <rpc_framework/server/rpc_router.h>

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
                dispatcher_->registerService<request_message::ServiceRequest>(public_data::MType::Req_service, std::bind(&rpc_registry::ProviderDiscovererManager::handleRegisterDiscoverRequest, pd_manager_.get(), std::placeholders::_1, std::placeholders::_2));

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

        // rpc服务器，既可以作为提供rpc服务的服务端，也可以作为发起服务注册的客户端
        class RpcServer
        {
        public:
            using ptr = std::shared_ptr<RpcServer>;
            // host_addr表示提供rpc服务的服务端需要的地址信息
            // registry_addr表示注册中心的地址信息
            // 是否需要启用服务注册功能取决于isToRegistry是否为true
            RpcServer(const public_data::host_addr_t &host_addr, bool isToRegistry = false, const public_data::host_addr_t &registry_addr = public_data::host_addr_t())
                :rpc_router_(std::make_shared<rpc_router::RpcRouter>()),
                dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>()),
                isToRegistry_(isToRegistry),
                host_addr_(host_addr)
            {
                // 向dispatcher模块注册rpc处理函数
                dispatcher_->registerService<request_message::RpcRequest>(public_data::MType::Req_rpc, std::bind(&rpc_router::RpcRouter::onRpcRequest, rpc_router_.get(), std::placeholders::_1, std::placeholders::_2));

                // 判断是否启用服务注册决定是否初始化服务注册客户端
                if(isToRegistry_)
                    reg_client_ = std::make_shared<rpc_client::main_client::RegisterClient>(registry_addr.first, registry_addr.second);

                // 创建服务端
                server_ = server_factory::ServerFactory::serverCreateFactory(host_addr.second);
                // 注册服务端的回调函数，由dispatcher提供
                server_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));
            }

            void start()
            {
                server_->start();
            }

            // 用于注册可以提供的服务
            void registryService(const rpc_router::ServiceDesc::ptr &s)
            {
                // 如果启用了服务注册，此时需要调用服务注册客户端的注册方法
                if(isToRegistry_)
                    reg_client_->toRegisterService(s->getMethodName(), host_addr_);

                // 向rpc_router模块中注册服务
                rpc_router_->registerService(s);
            }

        private:
            bool isToRegistry_; // 是否启用服务注册
            rpc_client::main_client::RegisterClient::ptr reg_client_; // 用于服务注册的客户端
            rpc_router::RpcRouter::ptr rpc_router_;                   // 用于处理RPC服务
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            base_server::BaseServer::ptr server_; // 用于处理rpc服务的服务端
            public_data::host_addr_t host_addr_; // 提供rpc服务的服务端信息
        };
    }
}

#endif