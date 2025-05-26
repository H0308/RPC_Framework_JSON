#ifndef __rpc_json_util_h__
#define __rpc_json_util_h__

#include <iostream>
#include <string>
#include "jsoncpp/json/json.h"
#include <rpc_framework/base/log.h>

namespace json_util
{
    using namespace log_system;
    class JsonUtil
    {
    public:
        // JSON字符串转换为普通字符串，外部需要传递JSON字符串
        static bool serialize(const Json::Value &json_object, std::string &json_str)
        {
            std::stringstream ss;          // 先实例化⼀个⼯⼚类对象
            Json::StreamWriterBuilder swb; // 通过⼯⼚类对象来⽣产派⽣类对象
            std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
            int ret = sw->write(json_object, &ss);
            if (ret != 0)
            {
                LOG(log_system::Level::Error, "JSON序列化失败");
                return false;
            }
            json_str = ss.str();
            return true;
        }

        // 普通字符串转换为JSON字符串，外部需要对JSON字符串进行处理
        static bool deserialize(const std::string &json_str, Json::Value &json_object)
        {
            // 实例化⼯⼚类对象
            Json::CharReaderBuilder crb;
            // ⽣产CharReader对象
            std::string errs;
            std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
            bool ret = cr->parse(json_str.c_str(), json_str.c_str() + json_str.size(), &json_object, &errs);
            if (ret == false)
            {
                LOG(Level::Error, "JSON反序列化失败: {}", errs);
                return false;
            }
            return true;
        }
    };
}

#endif