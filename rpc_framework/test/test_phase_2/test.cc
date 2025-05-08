#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/response_message.h>
#include <rpc_framework/factories/message_factory.h>

using namespace log_system;

// rpc请求和响应测试
void testRpc()
{
    // rpc请求
    request_message::RpcRequest::ptr rrq = message_factory::MessageFactory::messageCreateFactory<request_message::RpcRequest>();

    // rpc请求中需要指定方法和参数
    rrq->setMethod("add");
    Json::Value params;
    params["num1"] = 10;
    params["num2"] = 20;
    rrq->setParams(params);

    std::string json_str;
    if (!rrq->serialize(json_str))
    {
        LOG(Level::Error, "rpc请求序列化失败");
        return;
    }
    if (!rrq->check())
    {
        LOG(Level::Error, "rpc检查失败");
        return;
    }

    if (!rrq->deserialize(json_str))
    {
        LOG(Level::Error, "rpc请求反序列化失败");
        return;
    }

    std::cout << rrq->getMethod() << std::endl;
    std::cout << rrq->getParams()["num1"] << ":" << rrq->getParams()["num2"] << std::endl;

    // rpc响应
    response_message::RpcResponse::ptr rrsp = message_factory::MessageFactory::messageCreateFactory<response_message::RpcResponse>();

    rrsp->setRCode(public_data::RCode::RCode_fine);
    rrsp->setResult(10);

    if (!rrsp->serialize(json_str))
    {
        LOG(Level::Error, "rpc响应序列化失败");
        return;
    }
    if (!rrsp->check())
    {
        LOG(Level::Error, "rpc检查失败");
        return;
    }

    if (!rrsp->deserialize(json_str))
    {
        LOG(Level::Error, "rpc响应反序列化失败");
        return;
    }

    std::cout << static_cast<int>(rrsp->getRCode()) << std::endl;
    std::cout << rrsp->getResult() << std::endl;
}

// 主题请求测试
void testTopicReq()
{
    // 主题请求
    request_message::TopicRequest::ptr ssq = message_factory::MessageFactory::messageCreateFactory<request_message::TopicRequest>();

    // 不携带主题信息
    ssq->setTopicName("music");
    ssq->setTopicOptype(public_data::TopicOptype::Topic_create);
    std::string json_str;
    if (!ssq->serialize(json_str))
    {
        LOG(Level::Error, "主题请求序列化失败");
        return;
    }
    if (!ssq->check())
    {
        LOG(Level::Error, "主题请求检查失败");
        return;
    }

    if (!ssq->deserialize(json_str))
    {
        LOG(Level::Error, "主题请求反序列化失败");
        return;
    }

    std::cout << ssq->getTopicName() << std::endl;
    std::cout << static_cast<int>(ssq->getTopicOptype()) << std::endl;

    // 携带主题信息
    request_message::TopicRequest::ptr ssq1 = message_factory::MessageFactory::messageCreateFactory<request_message::TopicRequest>();

    ssq1->setTopicName("art");
    ssq1->setTopicOptype(public_data::TopicOptype::Topic_publish);
    ssq1->setMessage("hello world");

    json_str.clear();
    if (!ssq1->serialize(json_str))
    {
        LOG(Level::Error, "主题请求序列化失败");
        return;
    }
    if (!ssq1->check())
    {
        LOG(Level::Error, "主题请求检查失败");
        return;
    }

    if (!ssq1->deserialize(json_str))
    {
        LOG(Level::Error, "主题请求反序列化失败");
        return;
    }

    std::cout << ssq1->getTopicName() << std::endl;
    std::cout << static_cast<int>(ssq1->getTopicOptype()) << std::endl;
    std::cout << ssq1->getMessage() << std::endl;
}

void testTopicResp()
{
    // 主题响应
    response_message::TopicResponse::ptr trsp = message_factory::MessageFactory::messageCreateFactory<response_message::TopicResponse>();

    trsp->setRCode(public_data::RCode::RCode_fine);

    std::string json_str;
    if (!trsp->serialize(json_str))
    {
        LOG(Level::Error, "rpc响应序列化失败");
        return;
    }
    if (!trsp->check())
    {
        LOG(Level::Error, "rpc检查失败");
        return;
    }

    if (!trsp->deserialize(json_str))
    {
        LOG(Level::Error, "rpc响应反序列化失败");
        return;
    }

    std::cout << static_cast<int>(trsp->getRCode()) << std::endl;
}

// 服务请求测试
void testServiceReq()
{
    // 服务请求
    request_message::ServiceRequest::ptr ssq = message_factory::MessageFactory::messageCreateFactory<request_message::ServiceRequest>();

    // 非服务发现请求
    ssq->setMethod("add");
    ssq->setServiceOptype(public_data::ServiceOptype::Service_online);
    ssq->setHost(public_data::host_addr_t("127.0.0.1", 8080));

    std::string json_str;
    if(!ssq->serialize(json_str))
    {
        LOG(Level::Error, "服务请求序列化失败");
        return;
    }
    if(!ssq->check())
    {
        LOG(Level::Error, "服务请求检查失败");
        return;
    }

    if(!ssq->deserialize(json_str))
    {
        LOG(Level::Error, "服务请求反序列化失败");
        return;
    }

    std::cout << ssq->getMethod() << std::endl;
    std::cout << static_cast<int>(ssq->getServiceOptye()) << std::endl;
    public_data::host_addr_t host = ssq->getHost();
    std::cout << host.first << ":" << host.second << std::endl;
}

// 服务响应测试
void testServiceResp()
{
    response_message::ServiceResponse::ptr ssp = message_factory::MessageFactory::messageCreateFactory<response_message::ServiceResponse>();

    ssp->setMethod("add");
    ssp->setRCode(public_data::RCode::RCode_fine);
    ssp->setServiceOptye(public_data::ServiceOptype::Service_discover);

    std::vector<public_data::host_addr_t> hosts;
    hosts.emplace_back("127.0.0.1", 8080);
    hosts.emplace_back("127.0.0.1", 8081);
    ssp->setHosts(hosts);

    std::string json_str;
    if(!ssp->serialize(json_str))
    {
        LOG(Level::Error, "服务响应序列化失败");
        return;
    }

    if(!ssp->check())
    {
        LOG(Level::Error, "服务响应检查失败");
        return;
    }

    if (!ssp->deserialize(json_str))
    {
        LOG(Level::Error, "服务响应反序列化失败");
        return;
    }

    std::cout << static_cast<int>(ssp->getRCode()) << std::endl;
    std::cout << ssp->getMethod() << std::endl;

    std::vector<public_data::host_addr_t> temp = ssp->getHosts();
    std::for_each(temp.begin(), temp.end(), [](public_data::host_addr_t h){
        std::cout << h.first << ":" << h.second << std::endl;
    });

}

int main()
{
    // testRpc();
    // testTopicReq();
    // testTopicResp();
    // testServiceReq();
    testServiceResp();
}