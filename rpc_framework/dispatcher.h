#ifndef __rpc_dispatcher_h
#define __rpc_dispatcher_h

#include <unordered_map>
#include <mutex>
#include <rpc_framework/base_message.h>
#include <rpc_framework/base_connection.h>
#include <rpc_framework/public_data.h>
#include <rpc_framework/log.h>

// 消息分发模块
// 根据指定的消息类型选择指定的回调函数进行处理
namespace dispatcher_rpc_framework
{
    using namespace log_system;

    // version1
    // 当前版本太过于简陋，上层使用时必须是BaseMessage而不能是其子类
    class Dispatcher
    {
    public:
        using ptr = std::shared_ptr<Dispatcher>;

        // 注册服务
        void registerService(public_data::MType& m, const public_data::messageCallback_t& cb)
        {  
            std::unique_lock<std::mutex> lock(mtx_);
            auto pos = type_calls.find(m);
            if(pos != type_calls.end())
            {
                LOG(Level::Warning, "已经存在指定的消息类型，插入失败");
                return;
            }
            type_calls.insert({m, cb});
        }

        // 根据操作类型执行回调
        void executeService(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr & msg)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            auto pos = type_calls.find(msg->getMtype());
            if (pos != type_calls.end())
            {
                LOG(Level::Warning, "不存在指定的消息类型，分发失败");
                con->shutdown();
                return;
            }

            (pos->second)(con, msg);
        }

    private:
        std::unordered_map<public_data::MType, public_data::messageCallback_t> type_calls; // 消息类型和回调函数的映射
        std::mutex mtx_;                                                                   // 管理哈希表的互斥锁
    };
}

#endif