#ifndef __rpc_dispatcher_h__
#define __rpc_dispatcher_h__

#include <unordered_map>
#include <mutex>
#include <rpc_framework/base/base_message.h>
#include <rpc_framework/base/base_connection.h>
#include <rpc_framework/base/public_data.h>
#include <rpc_framework/base/log.h>

// 消息分发模块
// 根据指定的消息类型选择指定的回调函数进行处理
namespace dispatcher_rpc_framework
{
    using namespace log_system;

    // version1
    // 当前版本太过于简陋，上层使用时必须是BaseMessage而不能是其子类
#if 0
    class Dispatcher
    {
    public:
        using ptr = std::shared_ptr<Dispatcher>;

        // 注册服务
        void registerService(const public_data::MType& m, const public_data::messageCallback_t& cb)
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
            if (pos == type_calls.end())
            {
                LOG(Level::Debug, "错误的消息类型为：{}", static_cast<int>(msg->getMtype()));
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
#endif

    // version2
    // 3. 但是现在还是存在一个问题：因为Callback是模板类，类似于直接使用模板参数，也是表示任意类型，所以依旧无法写入到容器中
    // 此时可以考虑使用多态的思路，当前的Callback就是一个子类，让其继承BaseCallback即可，而BaseCallback此时就是容器的参数类型
    class BaseCallback
    {
    public:
        using ptr = std::shared_ptr<BaseCallback>;
        virtual void excuteService(const base_connection::BaseConnection::ptr &conn, base_message::BaseMessage::ptr &msg) = 0;
    };
    template <class T>
    class Callback : public BaseCallback
    {
    public:
        using ptr = std::shared_ptr<Callback>;
        // 6. 考虑设置一个回调函数类型，第二个参数就是具体子类对象类型
        using callback_t = std::function<void(const base_connection::BaseConnection::ptr &, std::shared_ptr<T> &)>;
        Callback(callback_t handler)
            : handler_(handler)
        {
        }

        void excuteService(const base_connection::BaseConnection::ptr &conn, base_message::BaseMessage::ptr &msg)
        {
            auto type_msg = std::dynamic_pointer_cast<T>(msg);
            handler_(conn, type_msg);
        }

    private:
        callback_t handler_;
    };

    // 2. 既然要存储特定的类型，那么可以考虑定义出一个具体的类型，例如Callback类
    // 这个类需要保存的内容就是任意类型的回调函数，此时该类也需要使用到模板
    // 而具体的函数只有两个，一个是构造函数，用于初始化对应的回调函数，另外一个就是用于注册的函数
    // 用于注册的函数内部就是实现通过模版参数转换为具体的子类对象，然后将具体的子类对象指针传递给设置的回调函数
    // template <class T>
    // class Callback
    // {
    // public:
    //     Callback(T handler)
    //         : handler_(handler)
    //     {
    //     }
    //     void excuteService(const BaseConnection::ptr &conn, const public_data::messageCallback_t &cb)
    //     {
    //         auto msg = std::dynamic_pointer_cast<T>(msg);
    //         handler_(conn, type_msg);
    //     }
    // private:
    //     T handler_;
    // };

    class Dispatcher
    {
    public:
        using ptr = std::shared_ptr<Dispatcher>;
        // 注册服务
        // 1. 要保证参数为BaseMessage子类对象指针的函数可以作为参数传递给registerService函数就必须确保registerService可以接收任意类型
        // 此时在registerService函数上方就需要使用模版，如下：
        // template <class T>
        // void registerService(const public_data::MType &m, const T &cb)
        // 但是这样实现的话，在插入cb对象时就会出现问题，因为容器是无法同时存储不同的类型的
        // 简单来说就是容器不能写为：unordered_map<Mtype, T>
        // 所以此时这种直接的方案就不行

        // 5. 修改当前参数使其支持模板
        // 但是在调用时，希望这个T是具体的请求/响应类型，而不是回调函数类型
        // 当前这种写法T只能表示回调函数的类型
        // template <class T>
        // void registerService(const public_data::MType &m, const T &cb)
        // {
        //     std::unique_lock<std::mutex> lock(mtx_);
        //     auto pos = type_calls.find(m);
        //     if (pos != type_calls.end())
        //     {
        //         LOG(Level::Warning, "已经存在指定的消息类型，插入失败");
        //         return;
        //     }

        //     // 创建出BaseCallback对象指针
        //     std::shared_ptr<BaseCallback> base_call = std::make_shared<Callback<T>>(cb);
        //     type_calls.insert({m, base_call});
        // }

        template <class T>
        void registerService(const public_data::MType &m, const typename Callback<T>::callback_t &cb)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            auto pos = type_calls.find(m);
            if (pos != type_calls.end())
            {
                LOG(Level::Warning, "已经存在指定的消息类型，插入失败");
                return;
            }

            // 创建出BaseCallback对象指针
            std::shared_ptr<BaseCallback> base_call = std::make_shared<Callback<T>>(cb);
            type_calls.insert({m, base_call});
        }

        // 根据操作类型执行回调
        void executeService(const base_connection::BaseConnection::ptr &con, base_message::BaseMessage::ptr &msg)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            auto pos = type_calls.find(msg->getMtype());
            if (pos == type_calls.end())
            {
                LOG(Level::Debug, "错误的消息类型为：{}", static_cast<int>(msg->getMtype()));
                LOG(Level::Warning, "不存在指定的消息类型，分发失败");
                con->shutdown();
                return;
            }

            (pos->second)->excuteService(con, msg);
        }

    private:
        // std::unordered_map<public_data::MType, public_data::messageCallback_t> type_calls; // 消息类型和回调函数的映射

        // 4. 此时容器的第二个类型就是BaseCallback
        std::unordered_map<public_data::MType, BaseCallback::ptr> type_calls; // 消息类型和回调函数的映射

        std::mutex mtx_; // 管理哈希表的互斥锁
    };
}

#endif