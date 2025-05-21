#ifndef __rpc_main_client_h__
#define __rpc_main_client_h__

#include <rpc_framework/client/requestor.h>
#include <rpc_framework/client/rpc_registry_client.h>
#include <rpc_framework/client/rpc_caller.h>
#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/base/base_client.h>
#include <rpc_framework/factories/client_factory.h>
#include <rpc_framework/client/rpc_topic_client.h>

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
            DiscovererClient(const std::string &ip, const uint16_t port, rpc_registry::Discoverer::offlineCallback_t cb)
                : requestor_(std::make_shared<requestor_rpc_framework::Requestor>()), discoverer_(std::make_shared<rpc_client::rpc_registry::Discoverer>(requestor_, cb)), dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>())
            {
                // 处理服务发现请求对应的响应
                dispatcher_->registerService<base_message::BaseMessage>(public_data::MType::Resp_service, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor_.get(), std::placeholders::_1, std::placeholders::_2));

                // 处理服务上线/下线请求对应的响应
                dispatcher_->registerService<request_message::ServiceRequest>(public_data::MType::Req_service, std::bind(&rpc_client::rpc_registry::Discoverer::handleOnlineOfflineServiceRequest, discoverer_.get(), std::placeholders::_1, std::placeholders::_2));

                client_ = client_factory::ClientFactory::clientCreateFactory(ip, port);
                client_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 连接服务端
                client_->connect();
            }
            bool toHandleDiscoveryRequest(const std::string &method, public_data::host_addr_t &host)
            {
                return discoverer_->handleDiscoveryRequest(client_->connection(), method, host);
            }

            void shutdown()
            {
                client_->shutdown();
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

            RpcClient(bool isToDiscover, const std::string &ip, const uint16_t port)
                : isToDiscover_(isToDiscover), requestor_(std::make_shared<requestor_rpc_framework::Requestor>()), dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>()),
                  rpc_caller_(std::make_shared<rpc_client::rpc_caller::RpcCaller>(requestor_))
            {
                // 处理RPC调用的回调
                dispatcher_->registerService<base_message::BaseMessage>(public_data::MType::Resp_rpc, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor_.get(), std::placeholders::_1, std::placeholders::_2));

                if (isToDiscover_)
                {
                    // 如果为真，说明此时需要进行服务提供者发现
                    // 此时的IP地址和端口表示的就是注册中心的IP地址和端口
                    // 同时绑定移除接口
                    discoverer_client_ = std::make_shared<DiscovererClient>(ip, port, std::bind(&RpcClient::removeClient, this, std::placeholders::_1));
                }
                else
                {
                    // 此时就是进行RPC调用
                    client_ = client_factory::ClientFactory::clientCreateFactory(ip, port);
                    client_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                    // 连接服务端
                    client_->connect();
                }
            }

            // 同步函数
            bool call(const std::string &method_name, const Json::Value &params, Json::Value &result)
            {
                // debug
                LOG(Level::Debug, "进入RpcClient的call同步函数");
                // 获取到指定的客户端调用
                base_client::BaseClient::ptr client = getClient(method_name);
                if (!client)
                {
                    LOG(Level::Warning, "获取客户端错误");
                    return false;
                }

                // 调用Rpc调用接口执行任务
                return rpc_caller_->call(client->connection(), method_name, params, result);
            }

            // 异步函数
            bool call(const std::string &method_name, const Json::Value &params, rpc_client::rpc_caller::RpcCaller::aysnc_response &result)
            {
                // 获取到指定的客户端调用
                base_client::BaseClient::ptr client = getClient(method_name);
                if (!client)
                {
                    LOG(Level::Warning, "获取客户端错误");
                    return false;
                }

                // 调用Rpc调用接口执行任务
                return rpc_caller_->call(client->connection(), method_name, params, result);
            }

            // 回调函数
            bool call(const std::string &method_name, const Json::Value &params, const rpc_client::rpc_caller::RpcCaller::callback_t &cb)
            {
                // 获取到指定的客户端调用
                base_client::BaseClient::ptr client = getClient(method_name);
                if (!client)
                {
                    LOG(Level::Warning, "获取客户端错误");
                    return false;
                }

                // 调用Rpc调用接口执行任务
                return rpc_caller_->call(client->connection(), method_name, params, cb);
            }

        private:
            // 找到合适的客户端调用接口
            base_client::BaseClient::ptr getClient(const std::string &method)
            {
                base_client::BaseClient::ptr client;
                // 判断是否需要发现客户端
                // 如果是发现客户端，则调用服务发现客户端的服务发现功能获取到客户端信息
                // 否则使用固定的客户端返回信息
                if (isToDiscover_)
                {
                    public_data::host_addr_t host;
                    bool ret = discoverer_client_->toHandleDiscoveryRequest(method, host);
                    if (!ret)
                    {
                        LOG(Level::Warning, "Rpc客户端服务发现失败");
                        return base_client::BaseClient::ptr();
                    }

                    // 判断是否已经存在对应的服务提供者
                    client = findClient(host);
                    if (!client)
                    {
                        // 不存在就创建
                        client = createClient(host);
                    }
                }
                else
                {
                    client = client_;
                }

                return client;
            }

            // 创建新客户端
            base_client::BaseClient::ptr createClient(const public_data::host_addr_t &host)
            {
                base_client::BaseClient::ptr client = client_factory::ClientFactory::clientCreateFactory(host.first, host.second);
                client->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 连接服务端
                client->connect();

                insertClient(host, client);

                return client;
            }

            // 对客户端集合进行增、删和获取
            void insertClient(const public_data::host_addr_t &host, const base_client::BaseClient::ptr &client)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                clients_.insert({host, client});
            }

            void removeClient(const public_data::host_addr_t &host)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                auto pos = clients_.find(host);
                if (pos == clients_.end())
                {
                    LOG(Level::Warning, "不存在指定的服务提供者，删除失败");
                    return;
                }

                clients_.erase(host);
            }

            base_client::BaseClient::ptr findClient(const public_data::host_addr_t &host)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                auto pos = clients_.find(host);
                if (pos == clients_.end())
                {
                    LOG(Level::Warning, "不存在指定的服务提供者，删除失败");
                    return base_client::BaseClient::ptr();
                }

                return pos->second;
            }

        private:
            // 自定义类型host_addr_t需要实现哈希函数，否则编译报错，因为unordered_map无法确定如何计算哈希值
            struct hostAddrHash
            {
                // 用于unordered_map的仿函数hash对象需要为实现const版本的operator()重载函数
                size_t operator()(const public_data::host_addr_t &h) const
                {
                    // 将主机地址信息转换为字符串类型便于使用std::hash计算哈希值
                    std::string host = h.first + std::to_string(h.second);
                    return std::hash<std::string>{}(host);
                }
            };
            bool isToDiscover_;                       // 是否需要进行服务发现
            DiscovererClient::ptr discoverer_client_; // 进行服务发现时启用服务发现客户端
            requestor_rpc_framework::Requestor::ptr requestor_;
            rpc_client::rpc_caller::RpcCaller::ptr rpc_caller_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            base_client::BaseClient::ptr client_;
            std::mutex manage_map_mtx_;
            std::unordered_map<public_data::host_addr_t, base_client::BaseClient::ptr, hostAddrHash> clients_; // 已经发现的所有可以提供指定RPC服务的客户端
        };

        class TopicClient
        {
        public:
            using ptr = std::shared_ptr<TopicClient>;
            TopicClient(const std::string &ip, const uint16_t port)
                : requestor_(std::make_shared<requestor_rpc_framework::Requestor>()), dispatcher_(std::make_shared<dispatcher_rpc_framework::Dispatcher>()), topic_manager_(std::make_shared<rpc_topic::TopicManager>())
            {
                // 处理主题响应的回调
                dispatcher_->registerService<base_message::BaseMessage>(public_data::MType::Resp_topic, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor_.get(), std::placeholders::_1, std::placeholders::_2));
                dispatcher_->registerService<request_message::TopicRequest>(public_data::MType::Req_topic, std::bind(&rpc_topic::TopicManager::handleTopicMessagePublishRequest, topic_manager_.get(), std::placeholders::_1, std::placeholders::_2));

                // 此时就是进行RPC调用
                client_ = client_factory::ClientFactory::clientCreateFactory(ip, port);
                client_->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher_.get(), std::placeholders::_1, std::placeholders::_2));

                // 连接服务端
                client_->connect();
            }

            // 新增主题
            bool createTopic(const std::string &topic_name)
            {
                return topic_manager_->createTopic(client_->connection(), topic_name);
            }

            // 删除主题
            bool removeTopic(const std::string &topic_name)
            {
                return topic_manager_->removeTopic(client_->connection(), topic_name);
            }

            // 订阅主题
            bool subscribeTopic(const std::string &topic_name, const rpc_topic::TopicManager::publishCallback &cb)
            {
                return topic_manager_->subscribeTopic(client_->connection(), topic_name, cb);
            }

            // 取消订阅主题
            bool cancelSubscribeTopic(const std::string &topic_name)
            {
                return topic_manager_->cancelSubscribeTopic(client_->connection(), topic_name);
            }

            // 主题发布
            bool publishTopicMessage(const std::string &topic_name, const std::string &content)
            {
                return topic_manager_->publishTopicMessage(client_->connection(), topic_name, content);
            }

        private:
            requestor_rpc_framework::Requestor::ptr requestor_;
            dispatcher_rpc_framework::Dispatcher::ptr dispatcher_;
            base_client::BaseClient::ptr client_;
            rpc_topic::TopicManager::ptr topic_manager_;
        };
    }
}

#endif