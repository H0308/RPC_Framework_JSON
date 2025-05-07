#include <iostream>
#include <rpc_framework/log.h>
#include <thread>
#include <rpc_framework/muduo_client.h>
#include <rpc_framework/factories/client_factory.h>

using namespace muduo_client;
using namespace log_system;

// 客户端收到消息时
void messageCallback(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
{
    std::string out_str;
    // 序列化得到结果打印
    if (!msg->serialize(out_str))
    {
        LOG(Level::Error, "序列化失败");
        return;
    }

    LOG(Level::Info, "序列化结果：\n{}", out_str);
}

int main()
{
    std::shared_ptr<base_client::BaseClient> mc = client_factory::ClientFactory::clientCreateFactory("127.0.0.1", 8080);
    mc->setMessageCallback(messageCallback);
    mc->connect();

    auto rpc_req = message_factory::MessageFactory::messageCreateFactory<request_message::RpcRequest>();
    rpc_req->setMethod("add");
    Json::Value root;
    root["num1"] = 10;
    root["num2"] = 20;
    rpc_req->setParams(root);
    rpc_req->setId("first");
    rpc_req->setMType(public_data::MType::Req_rpc);
    mc->send(rpc_req);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    mc->shutdown();

    // std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}