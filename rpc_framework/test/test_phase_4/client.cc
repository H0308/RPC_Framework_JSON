#include <rpc_framework/client/requestor.h>
#include <rpc_framework/client/rpc_caller.h>
#include <rpc_framework/base/dispatcher.h>
#include <rpc_framework/factories/client_factory.h>

using namespace log_system;

void handlerResult(const Json::Value& result)
{
    LOG(Level::Info, "计算结果为：{}", result.asInt());
}

int main()
{
    // 创建Requestor和RpcCaller对象
    auto requestor = std::make_shared<rpc_client::requestor_rpc_framework::Requestor>();
    auto rpc_caller = std::make_shared<rpc_client::rpc_caller::RpcCaller>(requestor);

    // 创建Dispatcher
    auto dispatcher = std::make_shared<dispatcher_rpc_framework::Dispatcher>();
    dispatcher->registerService<base_message::BaseMessage>(public_data::MType::Resp_rpc, std::bind(&rpc_client::requestor_rpc_framework::Requestor::handleResponse, requestor.get(), std::placeholders::_1, std::placeholders::_2));

    // 创建并启动客户端
    auto client = client_factory::ClientFactory::clientCreateFactory("127.0.0.1", 8080);
    client->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
    client->connect();

    // 组织数据
    auto con = client->connection();

    // 同步处理
    std::string method = "add";
    Json::Value params;
    params["num1"] = 20;
    params["num2"] = 30;
    Json::Value result1;
    bool ret = rpc_caller->call(con, method, params, result1);
    if(!ret)
    {
        LOG(Level::Error, "客户端RpcCaller调用错误");
        return 1;
    }

    LOG(Level::Info, "计算结果为：{}", result1.asInt());

    // 异步处理
    params["num1"] = 50;
    params["num2"] = 60;
    Json::Value result2;
    rpc_client::rpc_caller::RpcCaller::aysnc_response resp;
    ret = rpc_caller->call(con, method, params, resp);
    if (!ret)
    {
        LOG(Level::Error, "客户端RpcCaller调用错误");
        return 1;
    }
    result2 = resp.get();
    LOG(Level::Info, "计算结果为：{}", result2.asInt());

    // 回调处理
    params["num1"] = 70;
    params["num2"] = 90;

    ret = rpc_caller->call(con, method, params, handlerResult);
    if (!ret)
    {
        LOG(Level::Error, "客户端RpcCaller调用错误");
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    client->shutdown();

    return 0;
}