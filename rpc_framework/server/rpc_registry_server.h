#ifndef __rpc_rpc_registry_server_h__
#define __rpc_rpc_registry_server_h__

#include <set>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <unordered_map>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/utils/uuid_generator.h>

namespace rpc_server
{
    using namespace log_system;

    namespace rpc_registry
    {
        // 服务提供者信息
        struct ServiceProvider
        {
            using ptr = std::shared_ptr<ServiceProvider>;
            base_connection::BaseConnection::ptr con_; // 当前提供者的连接信息
            std::vector<std::string> methods_;         // 当前提供者可以提供的所有服务
            public_data::host_addr_t host_;            // 当前提供者的主机信息

            std::mutex method_mtx_; // 用于服务管理的线程安全

            ServiceProvider(const base_connection::BaseConnection::ptr &con, const public_data::host_addr_t &host)
                : con_(con), host_(host)
            {
            }

            void insertService(const std::string &method)
            {
                std::unique_lock<std::mutex> lock(method_mtx_);
                methods_.push_back(method);
            }
        };

        class ServiceProviderManager
        {
        public:
            using ptr = std::shared_ptr<ServiceProviderManager>;

            // 添加服务提供者
            void insertProvider(const base_connection::BaseConnection::ptr &con, const std::string &method, const public_data::host_addr_t &host)
            {
                ServiceProvider::ptr sp;
                {
                    std::unique_lock<std::mutex> lock(provider_mtx_);
                    // 建立连接和提供者之间的映射
                    // 查找是否存在对应的提供者
                    // auto pos = con_provider_.find(con);
                    // if (pos == con_provider_.end())
                    // {
                    //     // 不存在就添加
                    //     sp = std::make_shared<ServiceProvider>(con, host);
                    //     con_provider_.insert({con, sp});
                    // }
                    // else
                    // {
                    //     sp = pos->second; // 存在直接赋值
                    // }

                    // auto pos = con_provider_.insert({con, std::make_shared<ServiceProvider>(con, host)});
                    // 使用try_emplace可以先查找key是否存在再创建对象,insert会先构造对象再去判断是否存在
                    auto pos = con_provider_.try_emplace(con, std::make_shared<ServiceProvider>(con, host));
                    sp = pos.first->second;

                    // 找到指定服务对应的提供者映射数组，插入到该映射数组中
                    // 此处使用引用确保修改有效
                    auto &provider = providers_[method];
                    provider.insert(sp);
                }

                // 添加服务到当前提供者
                sp->insertService(method);
            }

            // 获取服务提供者
            ServiceProvider::ptr findProvider(const base_connection::BaseConnection::ptr &con)
            {
                std::unique_lock<std::mutex> lock(provider_mtx_);
                auto pos = con_provider_.find(con);
                if (pos == con_provider_.end())
                {
                    LOG(Level::Warning, "不存在指定的提供者");
                    return ServiceProvider::ptr();
                }

                return pos->second;
            }

            // 移除服务提供者
            void removeProvider(const base_connection::BaseConnection::ptr &con)
            {
                std::unique_lock<std::mutex> lock(provider_mtx_);
                auto pos = con_provider_.find(con);
                if (pos == con_provider_.end())
                {
                    LOG(Level::Warning, "不存在指定的提供者，删除失败");
                    return;
                }

                // 获取服务提供者，并根据其可以提供的方法将其从服务提供者和方法映射表中移除
                ServiceProvider::ptr sp = pos->second;
                std::vector<std::string> &methods = sp->methods_;

                for (std::string &m : methods)
                {
                    // 找到存储提供者的结构
                    auto &provider = providers_[m];
                    provider.erase(sp);
                }

                // 从管理连接和提供者的哈希表中删除
                con_provider_.erase(con);
            }

            // 根据指定服务获取可以提供该服务的所有服务提供者
            std::vector<public_data::host_addr_t> getServiceProviders(const std::string &method)
            {
                std::unique_lock<std::mutex> lock(provider_mtx_);
                auto pos = providers_.find(method);
                if (pos == providers_.end())
                {
                    LOG(Level::Warning, "不存在指定服务的提供者");
                    return std::vector<public_data::host_addr_t>();
                }

                std::vector<public_data::host_addr_t> hosts;
                for (auto &p : pos->second)
                    hosts.push_back(p->host_);

                return hosts;
            }

        private:
            std::mutex provider_mtx_;                                                                     // 用于提供者管理的线程安全
            std::unordered_map<std::string, std::set<ServiceProvider::ptr>> providers_;                   // 指定服务的所有提供者，使用set方便删除
            std::unordered_map<base_connection::BaseConnection::ptr, ServiceProvider::ptr> con_provider_; // 管理连接和提供者
        };

        // 服务发现者信息
        struct ServiceDiscoverer
        {
            using ptr = std::shared_ptr<ServiceDiscoverer>;

            base_connection::BaseConnection::ptr con_; // 发现过服务的客户端
            std::vector<std::string> methods_;         // 客户端发现的服务
            std::mutex method_mtx_;                    // 用于服务管理的线程安全

            ServiceDiscoverer(const base_connection::BaseConnection::ptr &con)
                : con_(con)
            {
            }

            void insertService(const std::string &method)
            {
                std::unique_lock<std::mutex> lock(method_mtx_);
                methods_.push_back(method);
            }
        };

        class ServiceDiscovererManager
        {
        public:
            using ptr = std::shared_ptr<ServiceDiscovererManager>;

            // 添加发现者
            void insertDiscoverer(const base_connection::BaseConnection::ptr &con, const std::string &method)
            {
                ServiceDiscoverer::ptr sd;
                {
                    std::unique_lock<std::mutex> lock(discover_mtx_);
                    // auto pos = con_provider_.find(con);
                    // if (pos == con_provider_.end())
                    // {
                    //     // 不存在，插入
                    //     sd = std::make_shared<ServiceDiscoverer>(con);
                    //     con_provider_.insert({con, sd});
                    // }
                    // else
                    // {
                    //     sd = pos->second; // 存在，获取
                    // }

                    auto pos = con_discoverer_.try_emplace(con, std::make_shared<ServiceDiscoverer>(con));
                    sd = pos.first->second;

                    // 获取所有客户端
                    auto &discover = discovers_[method];
                    discover.insert(sd);
                }

                sd->insertService(method);
            }

            // 移除发现者
            void removeDiscoverer(const base_connection::BaseConnection::ptr &con)
            {
                std::unique_lock<std::mutex> lock(discover_mtx_);
                auto pos = con_discoverer_.find(con);
                if (pos == con_discoverer_.end())
                {
                    LOG(Level::Info, "当前已经不存在任何发现者");
                    return;
                }

                ServiceDiscoverer::ptr sd = pos->second;
                auto &methods = sd->methods_;
                for (std::string &m : methods)
                {
                    auto &discovers = discovers_[m];
                    discovers.erase(sd);
                }

                con_discoverer_.erase(con);
            }

            // 服务上线提醒
            void onlineNotify(const std::string &method, const public_data::host_addr_t &addr)
            {
                notify(method, addr, public_data::ServiceOptype::Service_online);
            }

            // 服务下线提醒
            void offlineNotify(const std::string &method, const public_data::host_addr_t &addr)
            {
                notify(method, addr, public_data::ServiceOptype::Service_offline);
            }

        private:
            void notify(const std::string &method, const public_data::host_addr_t &addr, public_data::ServiceOptype op)
            {
                std::unique_lock<std::mutex> lock(discover_mtx_);
                auto pos = discovers_.find(method);
                if (pos == discovers_.end())
                {
                    LOG(Level::Warning, "当前服务：{}无提供者", method);
                    return;
                }

                // 构建服务发现请求并发送给所有客户端
                auto service_msg = message_factory::MessageFactory::messageCreateFactory<request_message::ServiceRequest>();
                service_msg->setId(uuid_generator::UuidGenerator::generate_uuid());
                service_msg->setMethod(method);
                service_msg->setHost(addr);
                service_msg->setServiceOptype(op);

                for (auto &d : pos->second)
                    d->con_->send(service_msg);
            }

        private:
            std::mutex discover_mtx_;             // 用于提供者管理的线程安全
            std::unordered_map<std::string, std::set<ServiceDiscoverer::ptr>> discovers_;                   // 发现指定服务的所有客户端
            std::unordered_map<base_connection::BaseConnection::ptr, ServiceDiscoverer::ptr> con_discoverer_; // 管理连接和发现者
        };

        class ProviderDiscovererManager
        {
        public:
            using ptr = std::shared_ptr<ProviderDiscovererManager>;

            ProviderDiscovererManager()
                : provider_manager_(std::make_shared<ServiceProviderManager>()),
                  discoverer_manager_(std::make_shared<ServiceDiscovererManager>())
            {
            }

            // 处理服务请求
            void handleRegisterDiscoverRequest(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
                // 收到服务请求只有两种情况：服务发现和服务注册
                // 对于服务上线和下线通知是当前服务端发送给客户端，需要由客户端进行处理的
                // 获取操作类型
                public_data::ServiceOptype type = msg->getServiceOptye();
                if (type == public_data::ServiceOptype::Service_register)
                {
                    // 服务注册
                    // 添加服务提供者到ProviderManager中
                    provider_manager_->insertProvider(con, msg->getMethod(), msg->getHost());
                    // 通知发现者
                    discoverer_manager_->onlineNotify(msg->getMethod(), msg->getHost());

                    sendRegistryResponse(con, msg);
                }
                else if (type == public_data::ServiceOptype::Service_discover)
                {
                    // 服务发现
                    discoverer_manager_->insertDiscoverer(con, msg->getMethod());

                    sendDiscoverResponse(con, msg);
                }
                else
                {
                    LOG(Level::Error, "收到服务请求，但是服务类型错误");
                    handleErrorResponse(con, msg);
                }
            }

            // 处理连接断开时
            void handleProviderConnectionShutdown(const base_connection::BaseConnection::ptr &con)
            {
                // 通知所有发现者
                auto sp = provider_manager_->findProvider(con);
                if (!sp)
                {
                    LOG(Level::Warning, "不存在指定的发现者，通知结束");
                    return;
                }
                for (auto &m : sp->methods_)
                    discoverer_manager_->offlineNotify(m, sp->host_);

                // 移除服务提供者
                provider_manager_->removeProvider(con);

                // 如果是服务发现者，移除
                discoverer_manager_->removeDiscoverer(con);
            }

        private:
            void handleErrorResponse(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
                auto service_resp = message_factory::MessageFactory::messageCreateFactory<response_message::ServiceResponse>();

                service_resp->setId(msg->getReqRespId());
                // 设置方法和主机信息
                service_resp->setMType(public_data::MType::Resp_service);
                service_resp->setServiceOptye(public_data::ServiceOptype::Service_wrong_type);
                service_resp->setRCode(public_data::RCode::RCode_not_found_service);

                con->send(service_resp);
            }

            void sendDiscoverResponse(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
                // 构建响应
                auto service_resp = message_factory::MessageFactory::messageCreateFactory<response_message::ServiceResponse>();
                // 获取主机信息
                auto hosts = provider_manager_->getServiceProviders(msg->getMethod());
                service_resp->setId(msg->getReqRespId());
                // 设置方法和主机信息
                service_resp->setMethod(msg->getMethod());
                service_resp->setMType(public_data::MType::Resp_service);
                service_resp->setServiceOptye(public_data::ServiceOptype::Service_discover);
                if (hosts.empty())
                {
                    LOG(Level::Warning, "不存在主机信息");
                    // 构建错误响应
                    service_resp->setRCode(public_data::RCode::RCode_not_found_service);
                    con->send(service_resp);
                    return;
                }

                service_resp->setRCode(public_data::RCode::RCode_fine);
                service_resp->setHosts(hosts);

                con->send(service_resp);
            }

            void sendRegistryResponse(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
                // 构建响应
                auto service_resp = message_factory::MessageFactory::messageCreateFactory<response_message::ServiceResponse>();
                // 获取主机信息
                service_resp->setId(msg->getReqRespId());
                // 设置方法和主机信息
                service_resp->setMType(public_data::MType::Resp_service);
                service_resp->setServiceOptye(public_data::ServiceOptype::Service_register);

                service_resp->setRCode(public_data::RCode::RCode_fine);

                con->send(service_resp);
            }

        private:
            ServiceProviderManager::ptr provider_manager_;
            ServiceDiscovererManager::ptr discoverer_manager_;
        };
    }
}

#endif