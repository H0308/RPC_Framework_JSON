#include <rpc_framework/server/rpc_router.h>
#include <rpc_framework/factories/server_factory.h>
#include <rpc_framework/base/dispatcher.h>

using namespace log_system;

void add(const Json::Value &params, Json::Value &result)
{
    int num1 = params["num1"].asInt();
    int num2 = params["num2"].asInt();

    result = num1 + num2;
}

int main()
{
    // debug
    LOG(Level::Debug, "进入服务");
    // 使用服务描述工厂创建服务
    std::unique_ptr<rpc_server::rpc_router::ServiceDescFactory> desc_factory = std::make_unique<rpc_server::rpc_router::ServiceDescFactory>();

    // 设置服务名称
    desc_factory->setMethodName("add");
    // 设置参数类型
    desc_factory->setParams("num1", rpc_server::rpc_router::params_type::Integral);
    desc_factory->setParams("num2", rpc_server::rpc_router::params_type::Integral);
    // 设置返回值类型
    desc_factory->setReturnType(rpc_server::rpc_router::params_type::Integral);

    // 设置回调函数——表示具体执行的服务
    desc_factory->setHandler(add);

    // 创建RpcRouter对象
    auto router = std::make_shared<rpc_server::rpc_router::RpcRouter>();
    // 注册可以提供的服务
    router->registerService(desc_factory->buildServiceDesc());

    // 将router中的处理请求的函数注册到Dispatcher中
    auto dispatcher = std::make_shared<dispatcher_rpc_framework::Dispatcher>();
    dispatcher->registerService<request_message::RpcRequest>(public_data::MType::Req_rpc, std::bind(&rpc_server::rpc_router::RpcRouter::onRpcRequest, router.get(), std::placeholders::_1, std::placeholders::_2));

    // 创建服务器并启动
    auto server = server_factory::ServerFactory::serverCreateFactory(8080);
    server->setMessageCallback(std::bind(&dispatcher_rpc_framework::Dispatcher::executeService, dispatcher.get(), std::placeholders::_1, std::placeholders::_2));
    server->start();

    return 0;
}