#ifndef __rpc_rpc_caller_h__
#define __rpc_rpc_caller_h__

#include <string>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/client/requestor.h>
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
            using callback_t = std::function<void(Json::Value&)>;

            RpcCaller(requestor_rpc_framework::Requestor::ptr requestor)
                : requestor_(requestor)
            {
            }

            // 同步调用函数
            void call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, Json::Value &result)
            {
            }

            // 异步调用函数
            void call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, aysnc_response &result)
            {
            }

            // 回调方式调用函数
            void call(const base_connection::BaseConnection::ptr &con, const std::string &method_name, const Json::Value &params, callback_t &cb)
            {
            }

        private:
            requestor_rpc_framework::Requestor::ptr requestor_; // 调用Requestor模块中的发送函数
        };
    }
}

#endif