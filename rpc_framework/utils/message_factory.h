#ifndef __rpc_message_factory_h__
#define __rpc_message_factory_h__

#include <rpc_framework/base_message.h>
#include <rpc_framework/request_message.h>

namespace message_factory
{
    // BaseMessage工厂，用于统一构造BaseMessage子类对象
    class MessageFactory
    {
    public:
        template<class T, class ...Args>
        static std::shared_ptr<T> messageCreateFactory(Args&& ...args)
        {
            return std::make_shared<T>(std::forward(args)...);
        }
    };
}

#endif