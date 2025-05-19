#ifndef __rpc_rpc_topic_h__
#define __rpc_rpc_topic_h__

#include <string>
#include <unordered_set>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/base_message.h>

namespace rpc_server
{
    namespace rpc_topic
    {
        // 主题管理者
        class TopicManager
        {
        public:
            using ptr = std::shared_ptr<TopicManager>;

            TopicManager()
            {
            }

            // 处理主题请求，注册到Dispatcher模块
            void handleTopicRequest(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
            }

            // 订阅者下线的处理
            void handleConnectionShutdown(const base_connection::BaseConnection::ptr &con)
            {
            }

        private:
            // 客户端连接和订阅的主题信息
            struct Subscriber
            {
                using ptr = std::shared_ptr<Subscriber>;

                base_connection::BaseConnection::ptr con_; // 客户端连接
                std::vector<std::string> topic_names_;     // 当前客户端订阅的所有主题
                std::mutex manage_vector_mtx_;             // 保证管理的线程安全

                // 主题创建（添加）
                void insertTopic(const std::string &topic_name)
                {
                }

                // 主题删除（移除）
                void removeTopic(const std::string &topic_name)
                {
                }
            };

            // 主题和订阅主题的发布者信息
            struct Topic
            {
                using ptr = std::shared_ptr<Topic>;

                std::string topic_name_;                                              // 主题名称
                std::unordered_set<base_connection::BaseConnection::ptr> subscibers_; // 所有订阅者
                std::mutex manage_set_mtx_;                                           // 保证管理的线程安全

                // 主题订阅（添加订阅者）
                void insertSubscriber(const Subscriber::ptr &con)
                {
                }

                // 主题取消订阅（移除订阅者）
                void removeSubscriber(const Subscriber::ptr &con)
                {
                }

                // 主题信息的发布
                void publicMessage(const base_message::BaseMessage::ptr &msg)
                {
                }
            };

        private:
            std::mutex manage_map_mtx;
            std::unordered_map<std::string, Topic::ptr> topics_; // 主题和订阅者的管理
            std::unordered_map<base_connection::BaseConnection::ptr, Subscriber::ptr> con_subscriber_; // 订阅者和连接映射
        };
    }
}

#endif