#ifndef __rpc_topic_client_h__
#define __rpc_topic_client_h__

#include <string>
#include <rpc_framework/client/requestor.h>
#include <rpc_framework/base/response_message.h>
#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/utils/uuid_generator.h>

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
            bool createTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name)
            {
                return baseRequest(con, topic_name, public_data::TopicOptype::Topic_create);
            }

            // 删除主题
            bool removeTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name)
            {
                return baseRequest(con, topic_name, public_data::TopicOptype::Topic_remove);
            }

            // 订阅主题
            bool subscribeTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name, const publishCallback &cb)
            {
                // 先插入回调函数，防止后续刚订阅主题就有消息需要处理
                insertCallback(topic_name, cb);
                return baseRequest(con, topic_name, public_data::TopicOptype::Topic_subscribe);
            }

            // 取消订阅主题
            bool cancelSubscribeTopic(const base_connection::BaseConnection::ptr &con, const std::string &topic_name)
            {
                // 先删除回调函数，确保取消之后都不处理消息
                removeCallback(topic_name);
                return baseRequest(con, topic_name, public_data::TopicOptype::Topic_cancel);
            }

            // 主题发布
            bool publishTopicMessage(const base_connection::BaseConnection::ptr &con, const std::string &topic_name, const std::string &content)
            {
                return baseRequest(con, topic_name, public_data::TopicOptype::Topic_publish, content);
            }

            // 处理收到发布的消息
            void handlerTopicMessagePublishResponse(const base_connection::BaseConnection::ptr &con, const request_message::TopicRequest::ptr &msg)
            {
                // 判断操作类型是否是主题消息发布
                public_data::TopicOptype topic_optype = msg->getTopicOptype();
                if(topic_optype != public_data::TopicOptype::Topic_publish)
                {
                    LOG(Level::Warning, "非主题消息发布操作类型，处理结束");
                    return;
                }

                // 获取到主题名称
                std::string topic_name = msg->getTopicName();
                // 获取到主题消息
                std::string topic_msg = msg->getMessage();

                // 根据主题名称获取到对应的回调函数
                const publishCallback publish_cb = findPublishCallback(topic_name);
                // 调用回调函数进行处理
                if(!publish_cb)
                {
                    LOG(Level::Warning, "主题{}对应的回调函数不存在", topic_name);
                    return;
                }

                publish_cb(topic_name, topic_msg);
            }

        private:
            // 通用的请求发送接口
            bool baseRequest(const base_connection::BaseConnection::ptr &con, const std::string &topic_name, const public_data::TopicOptype topic_optype, const std::string &content = "")
            {
                // 1. 构造出主题请求对象，并填充相关字段
                auto topic_req = message_factory::MessageFactory::messageCreateFactory<request_message::TopicRequest>();
                topic_req->setId(uuid_generator::UuidGenerator::generate_uuid());
                topic_req->setMType(public_data::MType::Req_topic);
                topic_req->setTopicName(topic_name);
                topic_req->setTopicOptype(topic_optype);
                // 如果操作类型是主题消息发布，则还需要设置发布的消息
                if (topic_optype == public_data::TopicOptype::Topic_publish)
                    topic_req->setMessage(content);

                // 2. 发送请求
                base_message::BaseMessage::ptr msg_resp;
                bool ret = requestor_->sendRequest(con, topic_req, msg_resp);
                if (!ret)
                {
                    LOG(Level::Warning, "主题操作请求发送失败");
                    return false;
                }

                // 3. 判断响应结果是否正确
                auto topic_resp = std::dynamic_pointer_cast<response_message::TopicResponse>(msg_resp);
                if (!topic_resp)
                {
                    LOG(Level::Warning, "向下转型失败");
                    return false;
                }
                if (topic_resp->getRCode() != public_data::RCode::RCode_fine)
                {
                    LOG(Level::Warning, "主题{}操作错误：{}", topic_name, public_data::errReason(topic_resp->getRCode()));
                    return false;
                }

                return true;
            }

            // 添加回调函数接口
            void insertCallback(const std::string &topic_name, const publishCallback &cb)
            {
                std::unique_lock<std::mutex> lock(manager_map_mtx_);
                topic_callback_.insert({topic_name, cb});
            }

            // 删除回调函数接口
            void removeCallback(const std::string &topic_name)
            {
                std::unique_lock<std::mutex> lock(manager_map_mtx_);
                topic_callback_.erase(topic_name);
            }

            // 获取回调函数接口
            const publishCallback &findPublishCallback(const std::string &topic_name)
            {
                std::unique_lock<std::mutex> lock(manager_map_mtx_);
                auto it = topic_callback_.find(topic_name);
                if(it == topic_callback_.end())
                {
                    LOG(Level::Warning, "不存在指定主题：{}对应的回调接口", topic_name);
                    return publishCallback();
                }

                return it->second;
            }

        private:
            requestor_rpc_framework::Requestor::ptr requestor_;
            std::mutex manager_map_mtx_;
            std::unordered_map<std::string, publishCallback> topic_callback_; // 不同的主题对应的处理回调函数映射
        };
    }
}

#endif