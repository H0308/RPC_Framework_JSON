#ifndef __rpc_rpc_caller_h__
#define __rpc_rpc_caller_h__

#include <string>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/client/requestor.h>
#include <rpc_framework/utils/uuid_generator.h>
#include "jsoncpp/json/value.h"

namespace rpc_client
{
    namespace rpc_caller
    {
        class RpcCaller
        {
        public:
            using ptr = std::shared_ptr<RpcCaller>;
            using aysnc_response = std::future<Json::Value>;
            using callback_t = std::function<void(const Json::Value &)>;

            RpcCaller(requestor_rpc_framework::Requestor::ptr requestor)
                : requestor_(requestor)
            {
            }

            // 同步调用函数
            bool call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, Json::Value &result)
            {
                // 1. 创建请求
                auto rpc_req = message_factory::MessageFactory::messageCreateFactory<request_message::RpcRequest>();
                rpc_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                rpc_req->setMType(public_data::MType::Req_rpc);
                rpc_req->setMethod(method_name);
                rpc_req->setParams(params);

                // 2. 发送请求
                base_message::BaseMessage::ptr base_msg;
                // 重载函数必须保证类型完全一致，而父类和子类之间的关系也属于类型不一致
                bool ret = requestor_->sendRequest(con, std::dynamic_pointer_cast<base_message::BaseMessage>(rpc_req), base_msg);
                if (!ret)
                {
                    LOG(Level::Warning, "同步处理请求失败");
                    return false;
                }

                // 3. 等待结果
                auto rpc_resp = std::dynamic_pointer_cast<response_message::RpcResponse>(base_msg);
                if (rpc_resp->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "结果异常，原因：{}", errReason(rpc_resp->getRCode()));
                    return false;
                }

                result = rpc_resp->getResult();
                return true;
            }

            // 异步调用函数
            bool call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, aysnc_response &result)
            {
                // 1. 创建请求
                auto rpc_req = message_factory::MessageFactory::messageCreateFactory<request_message::RpcRequest>();
                rpc_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                rpc_req->setMType(public_data::MType::Req_rpc);
                rpc_req->setMethod(method_name);
                rpc_req->setParams(params);

                // 2. 发送请求
                // 使用智能指针防止局部promise变量被销毁导致错误
                std::shared_ptr<std::promise<Json::Value>> json_promise = std::make_shared<std::promise<Json::Value>>();
                result = json_promise->get_future();
                requestor_rpc_framework::Requestor::callback_t cb = std::bind(&RpcCaller::async_callback, this, json_promise, std::placeholders::_1);
                // 重载函数必须保证类型完全一致，而父类和子类之间的关系也属于类型不一致
                bool ret = requestor_->sendRequest(con, std::dynamic_pointer_cast<base_message::BaseMessage>(rpc_req), cb);
                if (!ret)
                {
                    LOG(Level::Warning, "同步处理请求失败");
                    return false;
                }

                return true;
            }

            // 回调方式调用函数
            bool call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, callback_t &cb)
            {
                // 1. 创建请求
                auto rpc_req = message_factory::MessageFactory::messageCreateFactory<request_message::RpcRequest>();
                rpc_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                rpc_req->setMType(public_data::MType::Req_rpc);
                rpc_req->setMethod(method_name);
                rpc_req->setParams(params);

                // 设置回调函数
                requestor_rpc_framework::Requestor::callback_t req_cb = std::bind(&RpcCaller::cb_callback, this, cb, std::placeholders::_1);
                bool ret = requestor_->sendRequest(con, std::dynamic_pointer_cast<base_message::BaseMessage>(rpc_req), req_cb);
                if (!ret)
                {
                    LOG(Level::Warning, "同步处理请求失败");
                    return false;
                }

                return true;
            }

        private:
            // 回调请求函数
            void cb_callback(const callback_t &cb, base_message::BaseMessage::ptr &msg)
            {
                auto resp_rpc = std::dynamic_pointer_cast<response_message::RpcResponse>(msg);
                if (!resp_rpc)
                {
                    LOG(Level::Warning, "异步回调内部对象转换失败");
                    return;
                }

                if (resp_rpc->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "结果异常，原因：{}", errReason(resp_rpc->getRCode()));
                    return;
                }

                // 调用回调函数处理结果
                cb(resp_rpc->getResult());
            }

            // 异步请求回调函数
            void async_callback(std::shared_ptr<std::promise<Json::Value>> result, base_message::BaseMessage::ptr &msg)
            {
                auto resp_rpc = std::dynamic_pointer_cast<response_message::RpcResponse>(msg);
                if (!resp_rpc)
                {
                    LOG(Level::Warning, "异步回调内部对象转换失败");
                    return;
                }

                if (resp_rpc->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "结果异常，原因：{}", errReason(resp_rpc->getRCode()));
                    return;
                }
                result->set_value(resp_rpc->getResult());
            }

        private:
            requestor_rpc_framework::Requestor::ptr requestor_; // 调用Requestor模块中的发送函数
        };
    }
}

#endif