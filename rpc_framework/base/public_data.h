#ifndef __rpc_public_data_h__
#define __rpc_public_data_h__

#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <rpc_framework/base/base_message.h>
#include <rpc_framework/base/base_connection.h>

namespace public_data
{
    // 主机信息类型
    using host_addr_t = std::pair<std::string, uint16_t>;
    // 连接回调函数
    using connectionCallback_t = std::function<void(const base_connection::BaseConnection::ptr &)>;
    // 关闭连接时回调函数
    using closeCallback_t = std::function<void(const base_connection::BaseConnection::ptr &)>;
    // 收到消息时回调函数
    using messageCallback_t = std::function<void(const base_connection::BaseConnection::ptr &, base_message::BaseMessage::ptr &)>;

    // 最大64KB大小的数据
    const int max_data_size = (1 << 16);

// 请求和响应中body需要的字段
#define KEY_METHOD "method"       // 方法名
#define KEY_PARAMS "parameters"   // 方法参数
#define KEY_TOPIC_KEY "topic_key" // 主题名称
#define KEY_TOPIC_MSG "topic_msg" // 主题信息
#define KEY_OPTYPE "optype"       // 操作类型
#define KEY_HOST "host"           // 端口
#define KEY_HOST_IP "ip"          // IP地址
#define KEY_HOST_PORT "port"      // 端口号
#define KEY_RCODE "rcode"         // 返回状态码
#define KEY_RESULT "result"       // 返回值

    // 应用层协议中的消息类型
    enum class MType
    {
        Req_rpc = 0, // RPC请求
        Resp_rpc,    // RPC响应
        Req_topic,   // 主题请求
        Resp_topic,  // 主题响应
        Req_service, // 服务请求
        Resp_service // 服务响应
    };

    // 返回状态码
    enum class RCode
    {
        RCode_fine = 0,          // 正常
        RCode_parse_failed,      // 解析失败
        RCode_wrong_msgType,     // 错误的消息类型
        RCode_invalid_msg,       // 无效消息
        RCode_disconneted,       // 连接断开
        RCode_invalid_params,    // 错误参数
        RCode_not_found_service, // 未找到服务
        RCode_invalid_opType,    // 无效操作类型
        RCode_not_found_topic,   // 未找到主题
        RCode_internal_error     // 内部错误
    };

    // 获取错误原因字符串
    static std::string errReason(RCode code)
    {
        switch (code)
        {
        case RCode::RCode_fine:
            return "正常";
        case RCode::RCode_parse_failed:
            return "解析失败";
        case RCode::RCode_wrong_msgType:
            return "错误的消息类型";
        case RCode::RCode_invalid_msg:
            return "无效消息";
        case RCode::RCode_disconneted:
            return "连接断开";
        case RCode::RCode_invalid_params:
            return "错误参数";
        case RCode::RCode_not_found_service:
            return "未找到服务";
        case RCode::RCode_invalid_opType:
            return "无效操作类型";
        case RCode::RCode_not_found_topic:
            return "未找到主题";
        case RCode::RCode_internal_error:
            return "内部错误";
        default:
            return "无指定的错误原因";
        }

        return "";
    }

    // 消息发送模式
    enum class RType
    {
        Req_async = 0, // 异步模式
        Req_callback   // 回调模式
    };

    enum class TopicOptype
    {
        Topic_create = 0, // 主题创建
        Topic_remove,     // 主题移除
        Topic_subscribe,  // 主题订阅
        Topic_cancel,     // 主题取消
        Topic_publish     // 主题消息发布
    };

    enum class ServiceOptype
    {
        Service_register = 0, // 服务注册
        Service_discover,     // 服务发现
        Service_online,       // 服务上线
        Service_offline,      // 服务下线
        Service_wrong_type,   // 错误服务类型
        Service_unknown       // 不存在的服务类型
    };
}

#endif