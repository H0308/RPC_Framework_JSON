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
        class MethodHost
        {
        public:
            using ptr = std::shared_ptr<MethodHost>;

            MethodHost(const std::vector<public_data::host_addr_t> &hosts)
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
            Discoverer(const requestor_rpc_framework::Requestor::ptr &requestor)
                : requestor_(requestor)
            {
            }

            // 针对服务端发送的服务上线/下线请求处理
            void handleOnlineOfflineServiceRequest(const base_connection::BaseConnection::ptr &con, const request_message::ServiceRequest::ptr &msg)
            {
            }

            // 发起服务发现请求
            void sendDiscoveryRequest(const base_connection::BaseConnection::ptr &con, const std::string &method, public_data::host_addr_t &host)
            {
            }

        private:
            std::mutex manage_mtx_;
            std::unordered_map<std::string, MethodHost::ptr> service_providers_; // 每一个方法对应的所有提供者信息和方法映射表
            requestor_rpc_framework::Requestor::ptr requestor_;
        };
    }
}

#endif