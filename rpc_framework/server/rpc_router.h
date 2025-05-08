#ifndef __rpc_rpc_router_h__
#define __rpc_rpc_router_h__

#include <string>
#include <vector>
#include <functional>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/request_message.h>

namespace rpc_server
{
    namespace rpc_router
    {
        enum class params_type
        {
            Bool = 0,
            Integral,
            Numeric,
            String,
            Array,
            Object,
        };

        class ServiceDesc
        {
        public:
            using ptr = std::shared_ptr<ServiceDesc>;
            // 业务回调函数类型
            using handler_t = std::function<void(const Json::Value &, Json::Value &)>;
            using params_desciption_t = std::pair<std::string, params_type>;

            // 参数类型校验
            bool paramsCheck(const Json::Value &params)
            {
            }

        private:
            std::string method_name_;                // 方法名
            handler_t handler_;                      // 业务回调函数
            std::vector<params_desciption_t> params; // 保存所有参数和对应的类型
            params_type return_type_;                // 返回值类型
        };

        class ServiceManager
        {
        public:
            using ptr = std::shared_ptr<ServiceManager>;

            // 添加服务接口
            ServiceDesc::ptr insertService()
            {
            }

            // 删除服务接口
            void removeService()
            {
            }

            // 查找服务接口
            ServiceDesc::ptr findService()
            {
            }

        private:
            std::mutex manage_mtx_; // 用于管理的互斥锁
            std::unordered_map<std::string, ServiceDesc> service_manager_;
        };

        // 服务描述工厂
        class ServiceDescFactory
        {
        public:
            static ServiceDesc::ptr serviceDescFactory()
            {

            }
        };

        class RpcRouter
        {
        public:
            using ptr = std::shared_ptr<RpcRouter>;
            // 提供给Dispatcher模块的注册回调
            void onRpcRequest(const base_connection::BaseConnection::ptr &con, request_message::RpcRequest::ptr &msg)
            {
            }

            // 注册服务
            void registerService(const ServiceDesc &s)
            {
            }

        private:
            ServiceManager::ptr services_;
        };
    }
}

#endif