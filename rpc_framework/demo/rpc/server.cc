#include <rpc_framework/server/main_server.h>

using namespace log_system;

void add(const Json::Value &params, Json::Value &result)
{
    int num1 = params["num1"].asInt();
    int num2 = params["num2"].asInt();

    result = num1 + num2;
}

int main()
{
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

    rpc_server::main_server::RpcServer server(public_data::host_addr_t("127.0.0.1", 8080));
    server.registryService(desc_factory->buildServiceDesc());

    server.start();

    return 0;
}