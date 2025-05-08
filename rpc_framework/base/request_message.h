#ifndef __rpc_request_message_h__
#define __rpc_request_message_h__

#include <rpc_framework/base/json_message.h>

namespace request_message
{
    using namespace log_system;
    // Rpc请求类
    // 正文部分包含方法名和方法参数，而方法参数是一个Json对象
    class RpcRequest : public json_message::JsonRequest
    {
    public:
        using ptr = std::shared_ptr<RpcRequest>;
        // 实现检查方法
        virtual bool check() override
        {
            // 判断方法名是否存在且为字符串
            if (body_[KEY_METHOD].isNull() || !body_[KEY_METHOD].isString())
            {
                LOG(Level::Warning, "方法名错误");
                return false;
            }

            // 判断参数是否存在且为JSON对象
            if (body_[KEY_PARAMS].isNull() || !body_[KEY_PARAMS].isObject())
            {
                LOG(Level::Warning, "参数错误");
                return false;
            }

            return true;
        }

        // 设置和获取方法
        void setMethod(const std::string &m)
        {
            // method_ = m;
            body_[KEY_METHOD] = m;
        }

        std::string getMethod()
        {
            return body_[KEY_METHOD].asString();
        }

        // 设置和获取参数
        void setParams(const Json::Value &p)
        {
            // parameters_ = p;
            body_[KEY_PARAMS] = p;
        }

        Json::Value getParams()
        {
            return body_[KEY_PARAMS];
        }

    private:
        // 不需要成员，直接设置到正文JSON对象中
        // 因为下面的成员本质就是body_对象中的字段
        // 而不是单独出来的字段
        // std::string method_;
        // Json::Value parameters_;
    };

    // 主题请求类
    // 主要字段就是主题名称、主题操作类型和主题消息（只有操作类型为Topic_publish）才存在
    class TopicRequest : public json_message::JsonRequest
    {
    public:
        using ptr = std::shared_ptr<TopicRequest>;
        // 检查三个字段
        virtual bool check() override
        {
            // 检查主题名称
            // 判断主题是否存在且为字符串
            if (body_[KEY_TOPIC_KEY].isNull() || !body_[KEY_TOPIC_KEY].isString())
            {
                LOG(Level::Warning, "主题名称错误");
                return false;
            }

            // 检查主题操作类型
            // 判断是否存在操作类型且为整数
            if (body_[KEY_OPTYPE].isNull() || !body_[KEY_OPTYPE].isInt())
            {
                LOG(Level::Warning, "主题操作类型错误");
                return false;
            }

            // 检查消息
            // 判断是否为Topic_publish，如果是再检查是否存在消息且为字符串
            if (body_[KEY_OPTYPE].asInt() == static_cast<int>(public_data::TopicOptype::Topic_publish) &&
                (body_[KEY_TOPIC_MSG].isNull() || !body_[KEY_TOPIC_MSG].isString()))
            {
                LOG(Level::Warning, "主题消息错误");
                return false;
            }

            return true;
        }

        // 设置和获取主题名称
        void setTopicName(const std::string &n)
        {
            // name_ = n;
            body_[KEY_TOPIC_KEY] = n;
        }

        std::string getTopicName()
        {
            // return name_;
            return body_[KEY_TOPIC_KEY].asString();
        }

        // 设置和获取主题操作类型
        void setTopicOptype(const public_data::TopicOptype &op)
        {
            // op_ = op;
            body_[KEY_OPTYPE] = static_cast<int>(op);
        }

        public_data::TopicOptype getTopicOptype()
        {
            // return op_;
            return static_cast<public_data::TopicOptype>(body_[KEY_OPTYPE].asInt());
        }

        // 设置和获取主题信息
        void setMessage(const std::string &m)
        {
            // message_ = m;
            body_[KEY_TOPIC_MSG] = m;
        }

        std::string getMessage()
        {
            // return message_;
            return body_[KEY_TOPIC_MSG].asString();
        }

    private:
        // 不需要成员，直接设置到正文JSON对象中
        // std::string name_;
        // public_data::TopicOptype op_;
        // std::string message_;
    };

    // 服务请求类
    // 主要字段就是方法名、服务类型和主机信息
    class ServiceRequest : public json_message::JsonRequest
    {
    public:
        using ptr = std::shared_ptr<ServiceRequest>;
        // 检查字段
        virtual bool check() override
        {
            // 检查方法名
            // 判断方法名是否存在且为字符串
            if (body_[KEY_METHOD].isNull() || !body_[KEY_METHOD].isString())
            {
                LOG(Level::Warning, "方法名称错误");
                return false;
            }

            // 检查主题操作类型
            // 判断是否存在操作类型且为整数
            if (body_[KEY_OPTYPE].isNull() || !body_[KEY_OPTYPE].isInt())
            {
                LOG(Level::Warning, "服务操作类型错误");
                return false;
            }

            // 检查消息
            // 判断是否存在主机信息
            // ! 为什么是不等于而不是等于？因为服务发现是需要将主机信息作为结果返回给上层
            // ! 而对于其他服务类型来说，都需要在请求中携带自己的信息用于写入到注册中心
            if ((body_[KEY_OPTYPE].asInt() != static_cast<int>(public_data::ServiceOptype::Service_discover)) &&
                (body_[KEY_HOST].isNull() || !body_[KEY_HOST].isObject()) &&
                (body_[KEY_HOST][KEY_HOST_IP].isNull() || !body_[KEY_HOST][KEY_HOST_IP].isString()) &&
                (body_[KEY_HOST][KEY_HOST_PORT].isNull() || !body_[KEY_HOST][KEY_HOST_PORT].isInt()))
            {
                LOG(Level::Warning, "主机信息错误");
                return false;
            }

            return true;
        }

        // 设置/获取服务操作类型
        void setServiceOptype(const public_data::ServiceOptype so)
        {
            body_[KEY_OPTYPE] = static_cast<int>(so);
        }

        public_data::ServiceOptype getServiceOptye()
        {
            return static_cast<public_data::ServiceOptype>(body_[KEY_OPTYPE].asInt());
        }

        // 设置和获取方法名
        void setMethod(const std::string& n)
        {
            // name_ = n;
            body_[KEY_METHOD] = n;
        }

        std::string getMethod()
        {
            // return name_;
            return body_[KEY_METHOD].asString();
        }

        // 设置和获取服务操作类型
        void setHost(const public_data::host_addr_t &host)
        {
            // host_.first = host[KEY_HOST][KEY_HOST_IP].asString();
            // host_.second = host[KEY_HOST][KEY_HOST_PORT].asInt();
            // 以一个对象的方式插入到body_中
            Json::Value val;
            // ! 这里不需要指定host，直接指定IP和Port即可，再讲整体存入到host中
            val[KEY_HOST_IP] = host.first;
            val[KEY_HOST_PORT] = host.second;
            body_[KEY_HOST] = val;
        }

        public_data::host_addr_t getHost()
        {
            // return host_;
            public_data::host_addr_t host;
            host.first = body_[KEY_HOST][KEY_HOST_IP].asString();
            host.second = body_[KEY_HOST][KEY_HOST_PORT].asInt();

            return host;
        }

    private:
        // 不需要成员，直接设置到正文JSON对象中
        // std::string name_;              // 方法名
        // public_data::ServiceOptype op_; // 服务操作类型
        // public_data::host_addr_t host_; // 主机信息
    };
}

#endif