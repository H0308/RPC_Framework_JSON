#ifndef __rpc_protocal_factory_h__
#define __rpc_protocal_factory_h__

#include <rpc_framework/base_protocal.h>
#include <rpc_framework/length_value_protocal.h>

namespace protocal_factory
{
    class ProtocalFactory
    {
    public:
        template <class... Args>
        static base_protocal::BaseProtocol::ptr createProtocalFactory(Args &&...args)
        {
            return std::make_shared<length_value_protocal::LengthValueProtocal>(std::forward(args)...);
        }
    };
}

#endif