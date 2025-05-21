#include <rpc_framework/client/main_client.h>

using namespace log_system;

void topicCallback(const std::string &topic_name, const std::string &msg)
{
    LOG(Level::Info, "主题消息：{}", msg);
}

int main()
{
    auto client = std::make_shared<rpc_client::main_client::TopicClient>("127.0.0.1", 8080);
    // 创建主题
    bool ret = client->createTopic("new topic");
    if (!ret)
    {
        LOG(Level::Warning, "创建主题失败");
        return 1;
    }

    // 处理主题消息
    client->subscribeTopic("new topic", topicCallback);
    

    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
