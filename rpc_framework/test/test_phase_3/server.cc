#include <iostream>
#include <rpc_framework/log.h>
#include <rpc_framework/muduo_server.h>
#include <rpc_framework/response_message.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/factories/server_factory.h>

using namespace muduo_server;
using namespace log_system;

void messageCallback(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
{
    // 得到序列化结果便于查看
    std::string out_str;
    if (!msg->serialize(out_str))
    {
        LOG(Level::Error, "序列化失败");
        return;
    }

    // 给客户端回响应
    auto rpc_resp = message_factory::MessageFactory::messageCreateFactory<response_message::RpcResponse>();

    rpc_resp->setId("first");
    rpc_resp->setMType(public_data::MType::Resp_rpc);
    rpc_resp->setRCode(public_data::RCode::RCode_fine);
    rpc_resp->setResult(33);

    con->send(rpc_resp);

    LOG(Level::Info, "序列化结果：\n{}", out_str);
}

int main()
{
    std::shared_ptr<base_server::BaseServer> ms = server_factory::ServerFactory::serverCreateFactory(8080);
    ms->setMessageCallback(messageCallback);
    ms->start();

    return 0;
}