#include <rpc_framework/server/main_server.h>

// 启动注册中心
int main()
{  
    rpc_server::main_server::RegistryServer reg_server(9090);
    reg_server.start();

    return 0;
}