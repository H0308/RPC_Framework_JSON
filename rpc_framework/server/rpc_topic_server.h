#ifndef __rpc_rpc_topic_server_h__
#define __rpc_rpc_topic_server_h__

#include <string>
#include <unordered_set>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/request_message.h>
#include <rpc_framework/base/base_message.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/base/response_message.h>

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
                // 获取到主题操作类型
                public_data::TopicOptype topic_optype = msg->getTopicOptype();
                bool ret = true;
                switch (topic_optype)
                {
                case public_data::TopicOptype::Topic_create:
                    createTopic(con, msg); // 1. 主题创建
                    break;
                case public_data::TopicOptype::Topic_remove:
                    removeTopic(con, msg); // 2. 主题删除
                    break;
                case public_data::TopicOptype::Topic_subscribe:
                    ret = subscribeTopic(con, msg); // 3. 主题订阅
                    break;
                case public_data::TopicOptype::Topic_cancel:
                    cancelSubscribeTopic(con, msg); // 4. 主题取消订阅
                    break;
                case public_data::TopicOptype::Topic_publish:
                    ret = publishTopicMessage(con, msg); // 5. 主题发布
                    break;
                default:
                    sendErrorResponse(con, msg, public_data::RCode::RCode_invalid_opType);
                    break;
                }

                if(!ret)
                {
                    sendErrorResponse(con, msg, public_data::RCode::RCode_not_found_topic);
                    return;
                }
                
                sendTopicResponse(con, msg);
            }

            // 订阅者下线的处理
            void handleConnectionShutdown(const base_connection::BaseConnection::ptr &con)
            {
                // 根据下线的连接判断是否是订阅者
                // 如果是订阅者就根据订阅者关联的主题找到对应的Topic
                // 从Topic中的订阅者集合中移除下线的订阅者
                // 从连接和订阅者映射集合中移除订阅者
                Subscriber::ptr subscriber;
                std::unordered_set<Topic::ptr> topics;
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    auto it_sub = con_subscriber_.find(con);

                    subscriber = it_sub->second;
                    for(auto &topic_name : it_sub->second->topic_names_)
                    {
                        auto it_topic = topics_.find(topic_name);
                        topics.insert(it_topic->second);
                    }

                    con_subscriber_.erase(con);
                }
                for (auto &topic : topics)
                    topic->removeSubscriber(subscriber);
            }

        private:
            // 新增主题
            void createTopic(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 判断主题是否存在，如果不存在则创建新的Topic对象插入到集合中
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                std::string topic_name = msg->getTopicName();
                auto it = topics_.find(topic_name);
                if (it != topics_.end())
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
                std::unordered_set<Subscriber::ptr> subscribers;
                std::string topic_name = msg->getTopicName();
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
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
                for (auto &subscriber : subscribers)
                    subscriber->removeTopic(topic_name);
            }

            // 主题订阅
            bool subscribeTopic(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 找到指定的主题对应的Topic，向Topic中添加订阅者信息
                // 接着获取到指定的订阅者信息，向该订阅者管理的主题插入新增的主题信息
                Topic::ptr topic;
                Subscriber::ptr subscriber;
                std::string topic_name = msg->getTopicName();
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    auto it_topic = topics_.find(topic_name);
                    // 不存在则说明指定的主题不存在，此时需要报错防止后续行为异常
                    if (it_topic == topics_.end())
                    {
                        LOG(Level::Warning, "不存在指定的主题");
                        return false;
                    }

                    // 存在
                    topic = it_topic->second;

                    auto it_sub = con_subscriber_.find(con);
                    if (it_sub == con_subscriber_.end())
                    {
                        // 不存在则创建
                        subscriber = std::make_shared<Subscriber>(con);
                        con_subscriber_.insert({con, subscriber});
                    }
                    else
                    {
                        // 否则直接使用已有的
                        subscriber = it_sub->second;
                    }
                }

                if (topic && subscriber)
                {
                    topic->insertSubscriber(subscriber);
                    subscriber->insertTopic(topic_name);
                }

                return true;
            }

            // 取消订阅
            void cancelSubscribeTopic(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 找到指定主题的Topic，从管理的订阅者中删除指定的连接
                // 接着在指定的订阅者管理的主题集合中移除指定主题
                Topic::ptr topic;
                Subscriber::ptr subscriber;
                std::string topic_name = msg->getTopicName();
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    auto it_topic = topics_.find(topic_name);
                    // 不存在则说明指定的主题不存在，此时需要报错防止后续行为异常
                    if (it_topic != topics_.end())
                    {
                        // 存在直接使用，不存在不处理
                        topic = it_topic->second;
                    }

                    auto it_sub = con_subscriber_.find(con);
                    if (it_sub != con_subscriber_.end())
                    {
                        // 存在直接使用，不存在不处理
                        subscriber = it_sub->second;
                    }
                }

                if (topic && subscriber)
                {
                    topic->removeSubscriber(subscriber);
                    subscriber->removeTopic(topic_name);
                }
            }

            // 主题发布
            bool publishTopicMessage(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 找到指定主题对应的Topic，调用Topic中的消息发布接口发布消息
                Topic::ptr topic;
                std::string topic_name = msg->getTopicName();
                {
                    std::unique_lock<std::mutex> lock(manage_map_mtx_);
                    auto it_topic = topics_.find(topic_name);
                    if (it_topic == topics_.end())
                    {
                        LOG(Level::Warning, "消息发布失败，不存在对应的主题");
                        return false;
                    }

                    topic = it_topic->second;
                }

                if (topic)
                    topic->publicMessage(msg);

                return true;
            }

            // 发送主题操作响应
            void sendTopicResponse(const base_connection::BaseConnection::ptr &con, const base_message::BaseMessage::ptr &msg)
            {
                // 创建响应对象
                auto topic_resp = message_factory::MessageFactory::messageCreateFactory<response_message::TopicResponse>();
                // 设置字段
                topic_resp->setId(msg->getReqRespId());
                topic_resp->setMType(public_data::MType::Resp_topic);
                topic_resp->setRCode(public_data::RCode::RCode_fine);

                con->send(topic_resp);
            }

            // 错误主题操作响应
            void sendErrorResponse(const base_connection::BaseConnection::ptr &con, const base_message::BaseMessage::ptr &msg, public_data::RCode rcode)
            {
                // 创建响应对象
                auto topic_resp = message_factory::MessageFactory::messageCreateFactory<response_message::TopicResponse>();
                // 设置字段
                topic_resp->setId(msg->getReqRespId());
                topic_resp->setMType(public_data::MType::Resp_topic);
                topic_resp->setRCode(rcode);

                con->send(topic_resp);
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

                std::string topic_name_;                         // 主题名称
                std::unordered_set<Subscriber::ptr> subscibers_; // 所有订阅者
                std::mutex manage_set_mtx_;                      // 保证管理的线程安全

                // 主题订阅（添加订阅者）
                void insertSubscriber(const Subscriber::ptr &subsciber)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    subscibers_.insert(subsciber);
                }

                // 主题取消订阅（移除订阅者）
                void removeSubscriber(const Subscriber::ptr &subsciber)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    subscibers_.erase(subsciber);
                }

                // 主题信息的发布
                void publicMessage(const base_message::BaseMessage::ptr &msg)
                {
                    std::unique_lock<std::mutex> lock(manage_set_mtx_);
                    // 遍历连接集合发送消息
                    for (auto &subscriber : subscibers_)
                        if (subscriber)
                            subscriber->con_->send(msg);
                }
            };

        private:
            std::mutex manage_map_mtx_;
            std::unordered_map<std::string, Topic::ptr> topics_;                                       // 主题和订阅者的管理
            std::unordered_map<base_connection::BaseConnection::ptr, Subscriber::ptr> con_subscriber_; // 订阅者和连接映射
        };
    }
}

#endif