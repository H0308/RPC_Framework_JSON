#ifndef __rpc_protocal_factory_h__
#define __rpc_protocal_factory_h__

#include <rpc_framework/base/base_protocol.h>
#include <rpc_framework/base/length_value_protocol.h>

namespace protocol_factory
{
    class ProtocolFactory
    {
    public:
        template <class... Args>
        static base_protocol::BaseProtocol::ptr createProtocolFactory(Args &&...args)
        {
            return std::make_shared<length_value_protocol::LengthValueProtocol>(std::forward<Args>(args)...);
        }
    };
}

#endif