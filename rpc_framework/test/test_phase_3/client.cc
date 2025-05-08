#include <iostream>
#include <rpc_framework/base/log.h>
#include <thread>
#include <rpc_framework/base/muduo_client.h>
#include <rpc_framework/factories/client_factory.h>
#include <rpc_framework/base/dispatcher.h>

using namespace muduo_client;
using namespace log_system;

// 客户端收到rpc响应时
void messageCallback_rpc(const base_connection::BaseConnection::ptr &con, response_message::RpcResponse::ptr &msg)
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

// 客户端收到topic响应时
void messageCallback_topic(const base_connection::BaseConnection::ptr &con, response_message::TopicResponse::ptr &msg)
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
    std::shared_ptr<dispatcher_rpc_framework::Dispatcher> dp = std::make_shared<dispatcher_rpc_framework::Dispatcher>();
    std::shared_ptr<base_client::BaseClient> mc = client_factory::ClientFactory::clientCreateFactory("127.0.0.1", 8080);
    // 设置消息类型对应的回调函数
    // 针对rpc响应处理
    dp->registerService<response_message::RpcResponse>(public_data::MType::Resp_rpc, messageCallback_rpc);
    // 针对主题响应处理
    dp->registerService<response_message::TopicResponse>(public_data::MType::Resp_topic, messageCallback_topic);
    
    mc->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dp.get(), std::placeholders::_1, std::placeholders::_2));
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

    auto topic_req = message_factory::MessageFactory::messageCreateFactory<request_message::TopicRequest>();
    topic_req->setId("second");
    topic_req->setMType(public_data::MType::Req_topic);
    topic_req->setTopicName("music");
    topic_req->setTopicOptype(public_data::TopicOptype::Topic_create);
    
    mc->send(topic_req);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    mc->shutdown();

    // std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}