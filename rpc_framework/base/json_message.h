#ifndef __rpc_json_message_h__
#define __rpc_json_message_h__

#include "jsoncpp/json/json.h"
#include <rpc_framework/base/base_message.h>
#include <rpc_framework/base/log.h>
#include <rpc_framework/utils/JsonUtil.h>
#include <rpc_framework/base/public_data.h>

namespace json_message
{
    using namespace log_system;
    // 对BaseMessage进行进一步实现
    // 比BaseMessage部分多一个字段：正文
    // 但是因为正文具体内容需要由具体的子类实现
    // ! 为什么不对mtype和id进行序列化？
    class JsonMessage : public base_message::BaseMessage
    {
    public:
        using ptr = std::shared_ptr<JsonMessage>;
        // 序列化
        // 只实现对body进行序列化
        virtual bool serialize(std::string &msg) override
        {
            // 判断Json对象是否为空
            if (body_.isNull())
            {
                LOG(Level::Warning, "正文字段Body为空，序列化失败");
                return false;
            }

            // 调用Json工具类方法进行序列化
            if (!json_util::JsonUtil::serialize(body_, msg))
            {
                LOG(Level::Warning, "对Body序列化失败");
                return false;
            }

            return true;
        }

        // 反序列化
        // 只实现对body进行反序列化
        virtual bool deserialize(const std::string &msg) override
        {
            if (msg.empty())
            {
                LOG(Level::Warning, "反序列化失败，字符串为空");
                return false;
            }

            // 调用Json工具类方法进行反序列化
            if (!json_util::JsonUtil::deserialize(msg, body_))
            {
                LOG(Level::Warning, "反序列化失败");
                return false;
            }

            return true;
        }

        // ! JsonMessage不需要实现check函数
        // ! 因为在请求和响应中对字段的检查都有所不同，交给具体的子类实现

    protected:
        Json::Value body_;
    };

    // 请求类，不实现任何信息
    class JsonRequest : public JsonMessage
    {
    public:
        using ptr = std::shared_ptr<JsonRequest>;
        // ! JsonReqest不需要实现check函数
    };

    // 响应类，实现check函数，因为响应中大部分都是检查返回状态码
    class JsonResponse : public JsonMessage
    {
    public:
        using ptr = std::shared_ptr<JsonResponse>;
        // 检查返回值是否合法
        virtual bool check() override
        {
            // 判断返回值字段是否存在或者是否为整数，不是返回false
            if (body_[KEY_RCODE].isNull() || !body_[KEY_RCODE].isInt())
            {
                LOG(Level::Warning, "返回值类型错误");
                return false;
            }

            return true;
        }

        // 设置和返回状态码
        virtual void setRCode(const public_data::RCode r)
        {
            body_[KEY_RCODE] = static_cast<int>(r);
        }

        virtual public_data::RCode getRCode()
        {
            return static_cast<public_data::RCode>(body_[KEY_RCODE].asInt());
        }
    };
}

#endif