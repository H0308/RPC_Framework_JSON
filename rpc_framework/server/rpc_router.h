#ifndef __rpc_rpc_router_h__
#define __rpc_rpc_router_h__

#include <string>
#include <vector>
#include <functional>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/response_message.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/base/log.h>

namespace rpc_server
{
    using namespace log_system;
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

        // 业务回调函数类型
        using handler_t = std::function<void(const Json::Value &, Json::Value &)>;
        using params_desciption_t = std::pair<std::string, params_type>;

        class ServiceDesc
        {
        public:
            using ptr = std::shared_ptr<ServiceDesc>;

            // ServiceDesc() = default;

            // 右值不能带const
            ServiceDesc(std::string &&method, handler_t &&handler, std::vector<params_desciption_t> &&params, params_type &&return_type)
                : method_name_(std::move(method)), handler_(std::move(handler)), params_(params), return_type_(std::move(return_type))
            {
            }

            // 参数类型校验
            // 遍历指定的参数和类型与用户传递的参数和类型进行对比
            bool paramsCheck(const Json::Value &params)
            {
                for (const auto &desc : params_)
                {
                    // 判断字段是否存在
                    if (!params.isMember(desc.first))
                    {
                        LOG(Level::Warning, "指定的成员不存在：{}", desc.first);
                        return false;
                    }

                    // 判断字段类型和给定类型是否一致
                    // 注意需要判断的是指定的字段而不是直接params对象
                    if (!checkParamsType(desc.second, params[desc.first]))
                    {
                        // debug
                        // LOG(Level::Debug, "当前类型为：{}", params.asInt());
                        LOG(Level::Warning, "指定的类型错误：{}", static_cast<int>(desc.second));
                        return false;
                    }
                }

                return true;
            }

            // 调用回调函数
            bool callHandler(const Json::Value &input, Json::Value &output)
            {
                handler_(input, output);
                if (!checkReturnType(return_type_, output))
                {
                    LOG(Level::Warning, "返回值的类型错误：{}", static_cast<int>(return_type_));
                    return false;
                }

                return true;
            }

            // 获取服务名称
            std::string getMethodName()
            {
                return method_name_;
            }

        private:
            // 检查参数类型
            bool checkParamsType(const params_type &p, const Json::Value &val)
            {
                switch (p)
                {
                case params_type::Bool:
                    return val.isBool();
                case params_type::Integral:
                    return val.isIntegral();
                case params_type::Numeric:
                    return val.isNumeric();
                case params_type::String:
                    return val.isString();
                case params_type::Array:
                    return val.isArray();
                case params_type::Object:
                    return val.isObject();
                default:
                    break;
                }

                return false;
            }

            // 检查返回值类型
            bool checkReturnType(params_type return_type, const Json::Value &val)
            {
                return checkParamsType(return_type, val);
            }

        private:
            // friend class ServiceDescFactory;          // 建造者友元类
            std::string method_name_;                 // 方法名
            handler_t handler_;                       // 业务回调函数
            std::vector<params_desciption_t> params_; // 保存所有参数和对应的类型
            params_type return_type_;                 // 返回值类型
        };

        // 服务描述工厂
        // 使用简易的建造者模式：将修改方式放在工厂类中而不是提供给具体的类
        class ServiceDescFactory
        {
        public:
            void setMethodName(const std::string &name)
            {
                method_name_ = name;
            }

            void setHandler(const handler_t &handler)
            {
                handler_ = handler;
            }

            void setParams(const std::string &param, params_type pt)
            {
                params_.emplace_back(param, pt);
            }

            void setReturnType(const params_type &type)
            {
                return_type_ = type;
            }

            ServiceDesc::ptr buildServiceDesc()
            {
                return std::make_shared<ServiceDesc>(std::move(method_name_), std::move(handler_), std::move(params_), std::move(return_type_));
            }

        private:
            std::string method_name_;                 // 方法名
            handler_t handler_;                       // 业务回调函数
            std::vector<params_desciption_t> params_; // 保存所有参数和对应的类型
            params_type return_type_;                 // 返回值类型
        };

        // 使用友元类
#if 0
        class ServiceDescFactory
        {
        public:
            ServiceDescFactory()
                : desc_(std::make_shared<ServiceDesc>())
            {
            }

            void setMethodName(const std::string &name)
            {
                desc_->method_name_ = name;
            }

            void setHandler(const handler_t &handler)
            {
                desc_->handler_ = handler;
            }

            void setParams(const std::string &param, params_type pt)
            {
                desc_->params_.emplace_back(param, pt);
            }

            void setReturnType(const params_type &type)
            {
                desc_->return_type_ = type;
            }

            ServiceDesc::ptr buildServiceDesc()
            {
                return desc_;
            }

        private:
            ServiceDesc::ptr desc_;
        };
#endif

        class ServiceManager
        {
        public:
            using ptr = std::shared_ptr<ServiceManager>;

            // 添加服务接口
            void insertService(const ServiceDesc::ptr &desc)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                service_manager_.insert({desc->getMethodName(), desc});
            }

            // 删除服务接口
            void removeService(const ServiceDesc::ptr &desc)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                service_manager_.erase(desc->getMethodName());
            }

            // 查找服务接口
            ServiceDesc::ptr findService(const std::string &method)
            {
                std::unique_lock<std::mutex> lock(manage_mtx_);
                auto pos = service_manager_.find(method);
                if (pos == service_manager_.end())
                {
                    LOG(Level::Warning, "指定服务不存在：{}", method);
                    return nullptr;
                }

                return pos->second;
            }

        private:
            std::mutex manage_mtx_; // 用于管理的互斥锁
            std::unordered_map<std::string, ServiceDesc::ptr> service_manager_;
        };

        class RpcRouter
        {
        public:
            using ptr = std::shared_ptr<RpcRouter>;

            RpcRouter()
                : services_(std::make_shared<ServiceManager>())
            {
            }

            // 提供给Dispatcher模块的注册回调
            void handleRpcRequest(const base_connection::BaseConnection::ptr &con, request_message::RpcRequest::ptr &msg)
            {
                // 1. 查找请求服务是否存在
                auto pos = services_->findService(msg->getMethod());
                if (!pos)
                {
                    LOG(Level::Warning, "请求的：{} 服务不存在", msg->getMethod());
                    buildRpcResponse(con, msg, Json::Value(), public_data::RCode::RCode_not_found_service);
                }

                // 2. 判断请求中提供的参数是否正确
                if (!pos->paramsCheck(msg->getParams()))
                {
                    LOG(Level::Warning, "请求的：{} 服务参数错误", msg->getMethod());
                    buildRpcResponse(con, msg, Json::Value(), public_data::RCode::RCode_invalid_params);
                }

                // 3. 调用ServiceManager类中的函数执行服务
                Json::Value result;
                bool ret = pos->callHandler(msg->getParams(), result);
                if (!ret)
                {
                    LOG(Level::Warning, "请求的：{} 服务返回值错误（内部错误）", msg->getMethod());
                    buildRpcResponse(con, msg, Json::Value(), public_data::RCode::RCode_internal_error);
                }

                // 4. 返回处理结果
                buildRpcResponse(con, msg, result, public_data::RCode::RCode_fine);
            }

            // 注册服务
            // 调用ServiceManager类的插入函数
            void registerService(const ServiceDesc::ptr &s)
            {
                services_->insertService(s);
            }

        private:
            void buildRpcResponse(const base_connection::BaseConnection::ptr &con, request_message::RpcRequest::ptr &msg, const Json::Value &ret, public_data::RCode rcode)
            {
                // 构建RpcResponse对象并填充字段
                auto rpc_resp = message_factory::MessageFactory::messageCreateFactory<response_message::RpcResponse>();
                rpc_resp->setId(msg->getReqRespId());
                rpc_resp->setMType(public_data::MType::Resp_rpc);
                rpc_resp->setRCode(rcode);
                rpc_resp->setResult(ret);

                // 发送给客户端
                con->send(rpc_resp);
            }

        private:
            ServiceManager::ptr services_;
        };
    }
}

#endif