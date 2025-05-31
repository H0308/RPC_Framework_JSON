#include <rpc_framework/server/main_server.h>

int main()
{
    auto server = std::make_shared<rpc_server::main_server::TopicServer>(8080);
    server->start();

    return 0;
}