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
                :requestor_(requestor)
            {}

            // 服务注册接口——用于服务提供方
            // 主要行为就是通过Requestor模块向服务端发起服务注册请求
            bool registerService(const base_connection::BaseConnection::ptr &con, const std::string& method, const public_data::host_addr_t &host)
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
                if(!ret)
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
    }
}

#endif