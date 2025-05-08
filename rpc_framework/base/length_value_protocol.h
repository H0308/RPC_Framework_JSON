#ifndef __rpc_length_value_protocal_h__
#define __rpc_length_value_protocal_h__

#include <rpc_framework/factories/message_factory.h>
#include <rpc_framework/base/base_protocol.h>
#include <rpc_framework/base/log.h>
#include <arpa/inet.h>

namespace length_value_protocol
{
    using namespace log_system;

    const int32_t valid_length_field_length = 4;

    // LV格式的协议
    class LengthValueProtocol : public base_protocol::BaseProtocol
    {
    public:
        using ptr = std::shared_ptr<LengthValueProtocol>;
        // 判断是否是有效数据
        // 判断有效数据长度+有效数据长度字段的长度是否等于或者小于收到的总长度
        virtual bool canProcessed(const base_buffer::BaseBuffer::ptr &buf) override
        {
            if (buf->readableSize() < valid_length_field_length)
                return false;
            // debug
            // LOG(Level::Debug, "获取到的总长度：{}", buf->readableSize() - valid_length_field_length);
            // 尝试从获取到缓冲区前4个字节
            int32_t valid_length = buf->peekInt32();
            // 计算预期总长度
            int32_t expect_length = valid_length + valid_length_field_length;
            // 获取实际大小
            int32_t real_length = buf->readableSize();

            if (real_length < expect_length)
            {
                LOG(Level::Warning, "长度不足，无法处理");
                return false;
            }

            return true;
        }
        // 收到消息时的处理，从buffer中读取数据交给Message类处理
        virtual bool getContentFromProtocol(const base_buffer::BaseBuffer::ptr &buf, base_message::BaseMessage::ptr &msg) override
        {
            // 从缓冲区中获取每一个字段，默认已经判断数据可以处理
            // 即canProcessed返回true
            int32_t valid_length = buf->readInt32();
            int32_t mtype = buf->readInt32();
            int32_t id_length = buf->readInt32();
            std::string id = buf->retrieveAsString(id_length);
            // 正文部分，总长度-有效数据长度字段的长度-消息类型字段的长度-ID字段的长度-ID的长度
            std::string body = buf->retrieveAsString(valid_length - sizeof(mtype) - sizeof(id_length) - id.size());

            // 创建消息对象
            // 根据消息类型创建对象
            msg = message_factory::MessageFactory::messageCreateFactory(static_cast<public_data::MType>(mtype));
            if (!msg)
            {
                LOG(Level::Error, "根据消息类型创建消息对象指针失败，指针为空");
                return false;
            }

            // 对正文部分进行反序列化，将其中的JSON对象存储到成员body_中
            if (!msg->deserialize(body))
            {
                LOG(Level::Error, "正文部分反序列化失败");
                return false;
            }

            // 设置字段
            msg->setId(id);
            msg->setMType(static_cast<public_data::MType>(mtype));

            return true;
        }
        // 序列化接口，用于序列化Message类的成员
        virtual std::string constructProtocol(const base_message::BaseMessage::ptr &msg) override
        {
            // 对每一个字段序列化，需要注意网络字节序的转换，使用htonl
            std::string body_str;
            if (!msg->serialize(body_str))
            {
                LOG(Level::Error, "序列化失败");
                return "ErrorSerialize";
            }

            std::string id = msg->getReqRespId();
            auto mtype = htonl(static_cast<int32_t>(msg->getMtype()));
            int32_t id_len = htonl(id.size());
            int32_t h_total_len = sizeof(mtype) + sizeof(id_len) + id.size() + body_str.size();
            // 对总长度进行网络字节序转换
            int32_t n_total_len = htonl(h_total_len);

            // debug
            LOG(Level::Debug, "总长度：{}", h_total_len);

            std::string result;
            result.reserve(sizeof(n_total_len) + h_total_len); // 提前开辟空间，提高性能

            // 构建应用层协议
            // 使用二进制方式添加字段，而不是转换为字符，不能使用to_string
            result.append(reinterpret_cast<const char *>(&n_total_len), sizeof(n_total_len));
            result.append(reinterpret_cast<const char *>(&mtype), sizeof(mtype));
            result.append(reinterpret_cast<const char *>(&id_len), sizeof(id_len));
            result.append(id);
            result.append(body_str);

            return result;
        }
    };
}

#endif