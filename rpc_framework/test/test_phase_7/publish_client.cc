#include <rpc_framework/client/main_client.h>

using namespace log_system;

int main()
{
    auto client = std::make_shared<rpc_client::main_client::TopicClient>("127.0.0.1", 8080);
    // 创建主题
    bool ret = client->createTopic("new topic");
    if(!ret)
    {
        LOG(Level::Warning, "创建主题失败");
        return 1;
    }

    // 发布消息：10条消息
    for(int i = 0; i < 10; i++)
    {
        std::string msg = "new topic：" + std::to_string(i);
        client->publishTopicMessage("new topic", msg);
    }

    return 0;
}
