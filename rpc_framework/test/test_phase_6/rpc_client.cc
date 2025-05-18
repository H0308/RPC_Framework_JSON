#include <rpc_framework/client/main_client.h>

using namespace log_system;

void handlerResult(const Json::Value &result)
{
    LOG(Level::Info, "计算结果为：{}", result.asInt());
}

int main()
{
    // rpc_client::main_client::RpcClient client(false, "127.0.0.1", 8080);
    // 让客户端连接注册中心，再从注册中心获取到服务提供者
    rpc_client::main_client::RpcClient client(true, "127.0.0.1", 9090);

    // 同步处理
    std::string method = "add";
    Json::Value params;
    params["num1"] = 20;
    params["num2"] = 30;
    Json::Value result1;
    bool ret = client.call(method, params, result1);
    if (!ret)
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
    ret = client.call(method, params, resp);
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

    ret = client.call(method, params, handlerResult);
    if (!ret)
    {
        LOG(Level::Error, "客户端RpcCaller调用错误");
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));

    client.shutdown();

    return 0;
}