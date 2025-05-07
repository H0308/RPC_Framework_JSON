#include <iostream>
#include <rpc_framework/dispatcher.h>
#include <rpc_framework/log.h>
#include <rpc_framework/muduo_server.h>
#include <rpc_framework/response_message.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/factories/server_factory.h>

using namespace muduo_server;
using namespace log_system;

void messageCallback_rpc(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
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

void messageCallback_topic(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
{
    // 得到序列化结果便于查看
    std::string out_str;
    if (!msg->serialize(out_str))
    {
        LOG(Level::Error, "序列化失败");
        return;
    }

    // 给客户端回响应
    auto topic_resp = message_factory::MessageFactory::messageCreateFactory<response_message::TopicResponse>();

    topic_resp->setId("second");
    topic_resp->setMType(public_data::MType::Resp_topic);
    topic_resp->setRCode(public_data::RCode::RCode_fine);

    con->send(topic_resp);

    LOG(Level::Info, "序列化结果：\n{}", out_str);
}

int main()
{
    std::shared_ptr<dispatcher_rpc_framework::Dispatcher> dp = std::make_shared<dispatcher_rpc_framework::Dispatcher>();
    std::shared_ptr<base_server::BaseServer> ms = server_factory::ServerFactory::serverCreateFactory(8080);
    
    dp->registerService(public_data::MType::Req_rpc, messageCallback_rpc);
    dp->registerService(public_data::MType::Req_topic, messageCallback_topic);

    ms->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dp.get(), std::placeholders::_1, std::placeholders::_2));
    ms->start();

    return 0;
}