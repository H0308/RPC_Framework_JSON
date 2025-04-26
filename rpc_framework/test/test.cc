#include <iostream>
#include <rpc_framework/utils/JsonUtil.h>
#include <rpc_framework/utils/uuid_generator.h>

int main()
{
    // 创建一个JSON对象
    Json::Value json_obj;
    json_obj["name"] = "test";
    json_obj["id"] = 123;
    json_obj["is_valid"] = true;

    // 序列化为字符串
    std::string json_str;
    bool ret = json_util::JsonUtil::serialize(json_obj, json_str);

    if (ret)
    {
        std::cout << "序列化成功: " << json_str << std::endl;

        // 反序列化
        Json::Value parsed_json;
        if (json_util::JsonUtil::deserialize(json_str, parsed_json))
        {
            std::cout << "反序列化成功，name: " << parsed_json["name"].asString() << std::endl;
            std::cout << "id: " << parsed_json["id"].asInt() << std::endl;
            std::cout << "is_valid: " << std::boolalpha << parsed_json["is_valid"].asBool() << std::endl;
        }
    }

    // uuid生成
    std::string uuid = uuid_generator::Generator().generate_uuid();

    std::cout << uuid << std::endl;
    return 0;
}