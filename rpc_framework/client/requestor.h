#ifndef __rpc_requestor_h__
#define __rpc_requestor_h__

#include <future>
#include <functional>
#include <rpc_framework/base/public_data.h>
#include <rpc_framework/base/base_message.h>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/log.h>

namespace rpc_client
{
    using namespace log_system;

    namespace requestor_rpc_framework
    {
        class Requestor
        {
        public:
            using ptr = std::shared_ptr<Requestor>;
            // 异步结果类型
            using async_response = std::future<base_message::BaseMessage::ptr>;
            // 回调类型
            using callback_t = std::function<void(base_message::BaseMessage::ptr &)>;

            struct RequestDesc
            {
                using ptr = std::shared_ptr<RequestDesc>;

                base_message::BaseMessage::ptr request;                // 请求描述
                public_data::RType send_type;                          // 消息发送模式
                std::promise<base_message::BaseMessage::ptr> response; // 存储异步请求响应结果
                callback_t callback;                                   // 回调处理函数
            };

            // 收到服务端响应时的回调函数
            void handleResponse(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
            {
                // 1. 查找到指定的描述字段
                RequestDesc::ptr rd = findRequestDesc(msg->getReqRespId());
                if(!rd.get())
                {
                    LOG(Level::Warning, "不存在请求ID：{}对应的描述字段", msg->getReqRespId());
                    return;
                }

                // 2. 根据异步或者回调获取结果
                if(rd->send_type == public_data::RType::Req_async)
                    rd->response.set_value(msg);
                else if(rd->send_type == public_data::RType::Req_callback)
                    (rd->callback)(msg);

                // 3. 处理完当前响应后说明对应的请求结束，删除对应的rid
                removeRequestDesc(msg->getReqRespId());
            }

            // 同步发送接口
            bool sendRequest(const base_connection::BaseConnection::ptr &con, const base_message::BaseMessage::ptr &msg,  base_message::BaseMessage::ptr &resp)
            {
                // 创建出请求描述
                async_response resp_async;
                bool ret = sendRequest(con, msg, resp_async);
                if(!ret)
                {
                    LOG(Level::Error, "同步发送失败");
                    return false;
                }

                // 不存在结果时会阻塞
                resp = resp_async.get();

                return true;
            }

            // 异步发送接口
            bool sendRequest(const base_connection::BaseConnection::ptr &con, const base_message::BaseMessage::ptr &msg, async_response &resp)
            {
                // 创建出请求描述
                RequestDesc::ptr rd = insertRequestDesc(msg, msg->getReqRespId(), public_data::RType::Req_async);
                if(!rd.get())
                {
                    LOG(Level::Error, "异步发送创建请求描述失败");
                    return false;
                }

                // 发送请求
                con->send(msg);

                // 获取future对象
                resp = rd->response.get_future();

                return true;
            }

            // 回调发送接口
            bool sendRequest(const base_connection::BaseConnection::ptr &con, const base_message::BaseMessage::ptr &msg, callback_t &cb)
            {
                // 创建出请求描述
                RequestDesc::ptr rd = insertRequestDesc(msg, msg->getReqRespId(), public_data::RType::Req_callback, cb);
                if (!rd.get())
                {
                    LOG(Level::Error, "回调发送创建请求描述失败");
                    return false;
                }

                // 发送请求
                con->send(msg);
                
                return true;
            }
        private:
            // 添加请求描述
            RequestDesc::ptr insertRequestDesc(const base_message::BaseMessage::ptr &req, std::string rid, public_data::RType rtype, const callback_t &cb = nullptr)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                // 构建RequestDesc对象
                RequestDesc::ptr rd = std::make_shared<RequestDesc>();
                rd->request = req;
                rd->send_type = rtype;
                if(rtype == public_data::RType::Req_callback && cb)
                    rd->callback = cb;
                
                request_map_.insert({rid, rd});

                return rd;
            }   

            // 删除请求描述
            void removeRequestDesc(std::string rid)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                request_map_.erase(rid);
            }

            // 查找请求描述
            RequestDesc::ptr findRequestDesc(std::string rid)
            {
                std::unique_lock<std::mutex> lock(manage_map_mtx_);
                auto pos = request_map_.find(rid);
                if(pos == request_map_.end())
                {
                    LOG(Level::Warning, "不存在请求ID为：{}对应的请求描述", rid);
                    return nullptr;
                }

                return pos->second;
            }

        private:
            std::unordered_map<std::string, RequestDesc::ptr> request_map_; // 请求ID与描述映射
            std::mutex manage_map_mtx_;                                         // 用于管理哈希表的互斥锁
        };
    }
}

#endif