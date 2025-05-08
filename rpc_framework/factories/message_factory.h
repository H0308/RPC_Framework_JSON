#ifndef __rpc_message_factory_h__
#define __rpc_message_factory_h__

#include <rpc_framework/base/base_message.h>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/response_message.h>

namespace message_factory
{
    // BaseMessage工厂，用于统一构造BaseMessage子类对象
    class MessageFactory
    {
    public:
        // 根据消息类型确定
        static base_message::BaseMessage::ptr messageCreateFactory(public_data::MType mtype)
        {
            switch (mtype)
            {
            case public_data::MType::Req_rpc:
                return std::make_shared<request_message::RpcRequest>();
            case public_data::MType::Req_topic:
                return std::make_shared<request_message::TopicRequest>();
            case public_data::MType::Req_service:
                return std::make_shared<request_message::ServiceRequest>();
            case public_data::MType::Resp_rpc:
                return std::make_shared<response_message::RpcResponse>();
            case public_data::MType::Resp_topic:
                return std::make_shared<response_message::TopicResponse>();
            case public_data::MType::Resp_service:
                return std::make_shared<response_message::ServiceResponse>();
            }
            return base_message::BaseMessage::ptr(); // 相当于返回nullptr，即shared_ptr<base_message::BaseMessage>();
        }

        // 泛型版本
        template<class T, class ...Args>
        static std::shared_ptr<T> messageCreateFactory(Args&& ...args)
        {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }
    };
}

#endif