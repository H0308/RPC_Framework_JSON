#ifndef __rpc_response_message_h__
#define __rpc_response_message_h__

#include <rpc_framework/json_message.h>

namespace response_message
{
    using namespace log_system;
    class RpcResponse : public json_message::JsonResponse
    {
    public:
        using ptr = std::shared_ptr<RpcResponse>;

        // ! 需要重新实现check
        virtual bool check() override
        {
            // 判断返回状态码
            if (body_[KEY_RCODE].isNull() || !body_[KEY_RCODE].isInt())
            {
                LOG(Level::Warning, "返回状态码错误");
                return false;
            }

            // 判断返回值
            // 因为返回值可能不止一种类型，所以此处不判断返回值的类型是否正确
            // 而是交给上层进行处理
            if (body_[KEY_RESULT].isNull())
            {
                LOG(Level::Warning, "返回值错误");
                return false;
            }

            return true;
        }

        // ! 只需要实现setResult/getResult，setRCode/getRCode在父类已经有了
        // 获取和设置返回值
        void setResult(const Json::Value &v)
        {
            body_[KEY_RESULT] = v;
        }

        Json::Value getResult()
        {
            return body_[KEY_RESULT];
        }
    };

    class TopicResponse : public json_message::JsonResponse
    {
    public:
        using ptr = std::shared_ptr<TopicResponse>;
        // 不需要实现check和set/get方法
    };

    class ServiceResponse : public json_message::JsonResponse
    {
    public:
        using ptr = std::shared_ptr<ServiceResponse>;

        // ! 需要实现check
        virtual bool check() override
        {
            // 判断返回状态码
            if (body_[KEY_RCODE].isNull() || !body_[KEY_RCODE].isInt())
            {
                LOG(Level::Warning, "返回状态码错误");
                return false;
            }

            if (body_[KEY_OPTYPE].isNull() || !body_[KEY_OPTYPE].isInt())
            {
                LOG(Level::Warning, "服务操作类型错误");
                return false;
            }

            // 判断操作类型是否存在且为Service_discover
            // 如果存在需要判断是否存在方法名和主机信息数组
            if ((body_[KEY_OPTYPE].isInt() == static_cast<int>(public_data::ServiceOptype::Service_discover)) &&
                (body_[KEY_METHOD].isNull() || !body_[KEY_METHOD].isString()) &&
                (body_[KEY_HOST][KEY_HOST_IP].isNull() || !body_[KEY_HOST][KEY_HOST_IP].isString()) &&
                (body_[KEY_HOST][KEY_HOST_PORT].isNull() || !body_[KEY_HOST][KEY_HOST_PORT].isInt()))
            {
                LOG(Level::Warning, "操作类型为Service_discover，但是返回值错误");
                return false;
            }

            return true;
        }

        // 设置/获取服务类型
        public_data::ServiceOptype getServiceOptype()
        {
            return static_cast<public_data::ServiceOptype>(body_[KEY_OPTYPE].asInt());
        }

        void setServiceOptye(const public_data::ServiceOptype& o)
        {
            body_[KEY_OPTYPE] = static_cast<int>(o);
        }

        // 设置/获取方法名和主机信息
        void setMethod(const std::string &name)
        {
            body_[KEY_METHOD] = name;
        }

        std::string getMethod()
        {
            return body_[KEY_METHOD].asString();
        }

        // ! 注意主机信息是一个数组
        void setHosts(const std::vector<public_data::host_addr_t> &hosts)
        {
            std::for_each(hosts.begin(), hosts.end(), [this](public_data::host_addr_t h)
            {
                Json::Value host;
                host[KEY_HOST_IP] = h.first;
                host[KEY_HOST_PORT] = h.second;

                body_[KEY_HOST].append(host); 
            });
        }

        std::vector<public_data::host_addr_t> getHosts()
        {
            std::vector<public_data::host_addr_t> hosts;
            int length = body_[KEY_HOST].size();
            for (int i = 0; i < length; i++)
                hosts.emplace_back(body_[KEY_HOST][i][KEY_HOST_IP].asString(),
                                   body_[KEY_HOST][i][KEY_HOST_PORT].asInt());

            return hosts;
        }
    };
}

#endif