#ifndef __rpc_rpc_registry_client_h__
#define __rpc_rpc_registry_client_h__

#include <rpc_framework/client/requestor.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/utils/uuid_generator.h>

namespace rpc_client
{
    using namespace log_system;

    namespace rpc_registry
    {
        class Provider
        {
        public:
            using ptr = std::shared_ptr<Provider>;

            Provider(const rpc_client::requestor_rpc_framework::Requestor::ptr &requestor)
                : requestor_(requestor)
            {
            }

            // 服务注册接口——用于服务提供方
            // 主要行为就是通过Requestor模块向服务端发起服务注册请求
            bool registerService(const base_connection::BaseConnection::ptr &con, const std::string &method, const public_data::host_addr_t &host)
            {
                // 创建服务注册请求并填充字段
                auto service_req = message_factory::MessageFactory::messageCreateFactory<request_message::ServiceRequest>();

                service_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                service_req->setMethod(method);
                service_req->setHost(host);
                service_req->setServiceOptype(public_data::ServiceOptype::Service_register);
                service_req->setMType(public_data::MType::Req_service);

                // 发送服务注册请求
                base_message::BaseMessage::ptr base_resp;
                bool ret = requestor_->sendRequest(con, service_req, base_resp);
                if (!ret)
                {
                    LOG(Level::Warning, "服务注册请求发送失败");
                    return false;
                }

                // 转换为具体子类结果判断返回值
                response_message::ServiceResponse::ptr service_resp = std::dynamic_pointer_cast<response_message::ServiceResponse>(base_resp);

                if (service_resp->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "服务注册请求返回值类型异常：{}", public_data::errReason(service_resp->getRCode()));
                    return false;
                }

                return true;
            }

        private:
            rpc_client::requestor_rpc_framework::Requestor::ptr requestor_;
        };

        // 使用RR轮询的策略请求服务，提供指定函数的提供者信息和对应下标的结构
        class HostManager
        {
        public:
            using ptr = std::shared_ptr<HostManager>;

            HostManager(const std::vector<public_data::host_addr_t> &hosts = std::vector<public_data::host_addr_t>())
                : hosts_(hosts), index_(0)
            {
            }

            void insertHost(const public_data::host_addr_t &host)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                hosts_.push_back(host);
            }

            public_data::host_addr_t choostHost()
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                auto pos = (index_++) % hosts_.size();

                return hosts_[pos];
            }

            void removeHost(const public_data::host_addr_t &host)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);

                auto it = hosts_.begin();
                std::vector<public_data::host_addr_t>::iterator pos;
                while (it != hosts_.end())
                {
                    if ((*it) == host)
                    {
                        pos = it;
                        break;
                    }
                }

                hosts_.erase(pos);
            }

            bool emptyHosts()
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                return hosts_.empty();
            }

        private:
            std::mutex manage_mtx_; // 保证主机地址信息的线程安全
            size_t index_;          // 当前选择的服务提供者对应的下标
            std::vector<public_data::host_addr_t> hosts_;
        };

        class Discoverer
        {
        public:
            using ptr = std::shared_ptr<Discoverer>;
            using offlineCallback_t = std::function<void(const public_data::host_addr_t&)>;

            Discoverer(const requestor_rpc_framework::Requestor::ptr &requestor, const offlineCallback_t &cb)
                : requestor_(requestor), offline_cb_(cb)
            {
            }

            // 针对服务端发送的服务上线/下线请求处理
            void handleOnlineOfflineServiceRequest(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                // 只针对服务上线和下线的请求进行处理，不对其他服务类型的请求处理
                // 获取请求类型
                auto type = msg->getServiceOptye();
                auto method = msg->getMethod();
                if(type == public_data::ServiceOptype::Service_online)
                {
                    // 1. 服务上线请求处理
                    // 处理思路：查找是否存在指定服务的MethodHost对象
                    // 如果存在直接向其中添加
                    // 不存在则说明则构造一个该服务对应的MethodHost
                    // 再添加到哈希表中

                    LOG(Level::Info, "服务提供者：{}:{}上线了一个{}服务", msg->getHost().first, msg->getHost().second, msg->getMethod());
                    auto pos = service_providers_.find(method);
                    if (pos == service_providers_.end())
                    {
                        // 不存在指定服务
                        auto host = std::make_shared<HostManager>();
                        host->insertHost(msg->getHost());
                        service_providers_[method] = host;
                    }
                    else
                    {
                        // 存在直接添加
                        auto method_hosts = pos->second;
                        method_hosts->insertHost(msg->getHost());
                    }
                }
                else if(type == public_data::ServiceOptype::Service_offline)
                {
                    LOG(Level::Info, "服务提供者：{}:{}下线了一个{}服务", msg->getHost().first, msg->getHost().second, msg->getMethod());

                    // 2. 服务下线请求处理
                    // 将对应服务的主机从管理主机信息的结构中移除
                    auto pos = service_providers_.find(method);
                    if(pos == service_providers_.end())
                    {
                        LOG(Level::Warning, "不存在指定的服务");
                        return; 
                    }
                    auto method_hosts = pos->second;
                    auto host = msg->getHost();
                    method_hosts->removeHost(host);

                    // 在服务提供者下线时需要将该服务提供者从RpcClient的连接池中移除
                    if(offline_cb_)
                        offline_cb_(msg->getHost());
                }
            }

            // 进行服务发现
            bool discoverHost(const base_connection::BaseConnection::ptr &con, const std::string &method, public_data::host_addr_t &host)
            {
                {
                    std::unique_lock<std::mutex> lock(manage_mtx_);
                    // 判断是否存在指定的方法，如果存在，通过选择策略选择主机通过输出型参数返回给上层
                    auto pos = service_providers_.find(method);
                    if (pos != service_providers_.end())
                    {
                        // 判断主机信息管理结构是否为空
                        // 如果不为空，说明可以选择一个主机信息进行返回
                        auto method_host = pos->second;
                        if (!method_host->emptyHosts())
                        {
                            host = method_host->choostHost();
                            return true;
                        }
                    }
                }

                // 如果不存在指定的方法，那么肯定不存在对应的MethodHost结构
                // 此时就需要向服务端发起服务发现的请求
                auto service_req = message_factory::MessageFactory::messageCreateFactory<request_message::ServiceRequest>();
                service_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                service_req->setMethod(method);
                service_req->setMType(public_data::MType::Req_service);
                service_req->setServiceOptype(public_data::ServiceOptype::Service_discover);

                base_message::BaseMessage::ptr msg_resp;
                requestor_->sendRequest(con, service_req, msg_resp);

                auto service_resp = std::dynamic_pointer_cast<response_message::ServiceResponse>(msg_resp);
                if(!service_resp)
                {
                    LOG(Level::Warning, "向下转型失败");
                    return false;
                }

                if(service_resp->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "服务：{}发现错误：{}", method, public_data::errReason(service_resp->getRCode()));
                    return false;
                }

                std::unique_lock<std::mutex> lock(manage_mtx_);
                // 此时说明一定存在服务了
                // 构建MethodHost对象
                auto methodHost = std::make_shared<HostManager>(service_resp->getHosts());
                // 获取一个host返回
                host = methodHost->choostHost();
                // 插入到映射表
                service_providers_[method] = methodHost;

                return true;
            }

        private:
            std::mutex manage_mtx_;
            std::unordered_map<std::string, HostManager::ptr> service_providers_; // 每一个方法对应的所有提供者信息和方法映射表
            requestor_rpc_framework::Requestor::ptr requestor_;
            // 客户端离线时的处理回调
            offlineCallback_t offline_cb_;
        };
    }
}

#endif