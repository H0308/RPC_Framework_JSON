#ifndef __rpc_base_message_h__
#define __rpc_base_message_h__

#include <memory>
#include <rpc_framework/public_data.h>

namespace base_message
{
    using namespace public_data;

    // 消息抽象
    // 主要是包括了应用层协议中的请求/响应ID和消息类型
    // 该类作为JsonMessage类的基类
    // BaseMessage中的字段和正文部分一起组成应用层协议部分
    // 再派生出JsonRequest类（表示请求）和JsonResponse（表示响应）
    class BaseMessage
    {
    public:
        using ptr = std::shared_ptr<BaseMessage>;

        virtual ~BaseMessage() {}
        // 设置请求/响应ID
        virtual void setId(const std::string &id)
        {
            req_resp_id_ = id;
        }
        // 获取请求/响应ID
        virtual std::string getReqRespId()
        {
            return req_resp_id_;
        }
        // 设置消息类型
        virtual void setMType(MType mtype)
        {
            mtype_ = mtype;
        }
        // 获取消息类型
        virtual MType mtype()
        {
            return mtype_;
        }
        // 序列化
        virtual bool serialize(std::string &msg) = 0;
        // 反序列化
        virtual bool deserialize(const std::string &msg) = 0;
        // 检查消息是否合法
        virtual bool check() = 0;

    protected:
        MType mtype_;
        std::string req_resp_id_;
    };
}

#endif