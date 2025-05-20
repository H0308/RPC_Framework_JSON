#ifndef __rpc_topic_client_h__
#define __rpc_topic_client_h__

#include <string>
#include <rpc_framework/client/requestor.h>
#include <rpc_framework/base/response_message.h>

namespace rpc_client
{
    using namespace log_system;
    namespace rpc_topic
    {
        class TopicManager
        {
        public:
            using ptr = std::shared_ptr<TopicManager>;
            // 收到发布的消息时执行的回调函数类型
            using publishCallback = std::function<void(const std::string &topic_name, const std::string &msg)>;

            TopicManager(const requestor_rpc_framework::Requestor::ptr &requestor)
                : requestor_(requestor)
            {
            }

            // 新增主题
            bool insertTopic(const std::string &topic_name)
            {
            }

            // 删除主题
            bool removeTopic(const std::string &topic_name)
            {
            }

            // 订阅主题
            bool subscribeTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name, const publishCallback &cb)
            {
            }

            // 取消订阅主题
            bool cancelSubscribeTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name)
            {
            }

            // 主题发布
            bool publishTopicMessage(const base_connection::BaseConnection::ptr &con, const std::string &topic_name, const std::string &content)
            {
            }

            // 处理收到发布的消息
            void handlerTopicMessagePublishResponse(const base_connection::BaseConnection::ptr &con, const response_message::TopicResponse &msg)
            {
            }

        private:
            requestor_rpc_framework::Requestor::ptr requestor_;
            std::mutex manager_map_mtx_;
            std::unordered_map<std::string, publishCallback> topic_callback_; // 不同的主题对应的处理回调函数映射
        };
    }
}

#endif