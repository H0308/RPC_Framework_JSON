#ifndef __rpc_rpc_topic_h__
#define __rpc_rpc_topic_h__

#include <string>
#include <unordered_set>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/base_message.h>

namespace rpc_server
{
    using namespace log_system;
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
                // 1. 主题创建

                // 2. 主题删除
                // 3. 主题订阅
                // 4. 主题取消订阅
                // 5. 主题发布
            }

            // 订阅者下线的处理
            void handleConnectionShutdown(const base_connection::BaseConnection::ptr &con)
            {
            }

        private:
            // 新增主题
            void createTopic(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 判断主题是否存在，如果不存在则创建新的Topic对象插入到集合中
                std::unique_lock<std::mutex> lock(manage_map_mtx);
                std::string topic_name = msg->getTopicName();
                auto it = topics_.find(topic_name);
                if (it == topics_.end())
                {
                    LOG(Level::Warning, "指定主题已经存在");
                    return;
                }
                Topic::ptr topic = std::make_shared<Topic>(topic_name);
                topics_.insert({topic_name, topic});
            }

            // 主题删除
            void removeTopic(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 获取到指定的Topic对象指针
                std::unordered_set<base_connection::BaseConnection::ptr> subscribers;
                std::string topic_name = msg->getTopicName();
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx);
                    // 根据获取到的Topic找到其中的管理订阅者连接的集合
                    auto it_topic = topics_.find(topic_name);
                    if (it_topic == topics_.end())
                    {
                        LOG(Level::Warning, "不存在指定的主题");
                        return;
                    }
                    // 根据订阅者集合获取到订阅该主题的所有连接
                    subscribers = it_topic->second->subscibers_;
                    // 再从主题管理集合中移除该主题
                    topics_.erase(topic_name);
                }
                // 根据每一个连接找到对应的订阅者，在每一个订阅者中移除待删除的主题
                for (auto &con : subscribers)
                {
                    auto it_sub = con_subscriber_.find(con);
                    it_sub->second->removeTopic(topic_name);
                }
            }

        private:
            // 客户端连接和订阅的主题信息
            struct Subscriber
            {
                using ptr = std::shared_ptr<Subscriber>;

                Subscriber(const base_connection::BaseConnection::ptr &con)
                    : con_(con)
                {
                }

                base_connection::BaseConnection::ptr con_;    // 客户端连接
                std::unordered_set<std::string> topic_names_; // 当前客户端订阅的所有主题
                std::mutex manage_set_mtx_;                   // 保证管理的线程安全

                // 主题创建（添加）
                void insertTopic(const std::string &topic_name)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    auto it = topic_names_.find(topic_name);
                    if (it != topic_names_.end())
                    {
                        LOG(Level::Warning, "已存在指定的主题");
                        return;
                    }
                    topic_names_.insert(topic_name);
                }

                // 主题删除（移除）
                void removeTopic(const std::string &topic_name)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    auto it = topic_names_.find(topic_name);
                    if (it != topic_names_.end())
                    {
                        LOG(Level::Warning, "已存在指定的主题");
                        return;
                    }
                    topic_names_.erase(topic_name);
                }
            };

            // 主题和订阅主题的发布者信息
            struct Topic
            {
                using ptr = std::shared_ptr<Topic>;

                Topic(const std::string &topic_name)
                    : topic_name_(topic_name)
                {
                }

                std::string topic_name_;                                              // 主题名称
                std::unordered_set<base_connection::BaseConnection::ptr> subscibers_; // 所有订阅者
                std::mutex manage_set_mtx_;                                           // 保证管理的线程安全

                // 主题订阅（添加订阅者）
                void insertSubscriber(const base_connection::BaseConnection::ptr &subsciber)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    auto it = subscibers_.find(subsciber);
                    if (it != subscibers_.end())
                    {
                        LOG(Level::Warning, "已存在指定的订阅者");
                        return;
                    }
                    subscibers_.insert(subsciber);
                }

                // 主题取消订阅（移除订阅者）
                void removeSubscriber(const base_connection::BaseConnection::ptr &subsciber)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    auto it = subscibers_.find(subsciber);
                    if (it != subscibers_.end())
                    {
                        LOG(Level::Warning, "已存在指定的订阅者");
                        return;
                    }
                    subscibers_.erase(subsciber);
                }

                // 主题信息的发布
                void publicMessage(const base_message::BaseMessage::ptr &msg)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    // 遍历连接集合发送消息
                    for (auto &con : subscibers_)
                        if (con)
                            con->send(msg);
                }
            };

        private:
            std::mutex manage_map_mtx;
            std::unordered_map<std::string, Topic::ptr> topics_;                                       // 主题和订阅者的管理
            std::unordered_map<base_connection::BaseConnection::ptr, Subscriber::ptr> con_subscriber_; // 订阅者和连接映射
        };
    }
}

#endif