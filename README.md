# 基于JSON的RPC框架

## 简介

项目介绍：一个使用C++语言开发的分布式RPC（远程过程调用）框架，采用JSON作为数据序列化格式。该框架旨在简
化分布式环境下服务间的通信，提供包括远程方法调用、服务动态注册与发现、以及发布/订阅消息模型在内的核心功能

项目特点：
1. 模块化设计与JSON序列化：整体采用模块化设计，使用JSONCPP进行消息的序列化与反序列化，方便调试与扩展。
2. 动态服务治理：实现服务注册中心，支持服务的动态注册、发现、上线与下线通知，提高系统灵活性和可用性。
3. 发布/订阅消息系统：内建Topic机制，支持消息的发布与订阅，适用于异步通信和事件驱动场景。
4. 基于Muduo的高性能网络通信：利用Muduo网络库处理底层TCP连接与事件驱动，保证通信效率。
5. 灵活的调用方式：客户端支持同步、异步和回调请求调用，适应不同业务需求。

## 资源准备

1. C++17及以上
2. JSONCPP库
3. Muduo网络库
4. Boost库
5. spdlog日志库

> 项目资源准备自行完成

## 使用

### RPC基础功能

```shell
git clone https://github.com/H0308/RPC_Framework_JSON.git
cd RPC_Framework_JSON/rpc_framework/demo/rpc
# 先修改Makefile中有关资源路径的配置
make
./server
./client
```

### 结合注册中心的RPC功能

```shell
git clone https://github.com/H0308/RPC_Framework_JSON.git
cd RPC_Framework_JSON/rpc_framework/demo/rpc_with_register
# 先修改Makefile中有关资源路径的配置
make
./registry_server
./rpc_server
./rpc_client
```

### 主题功能

```shell
git clone https://github.com/H0308/RPC_Framework_JSON.git
cd RPC_Framework_JSON/rpc_framework/demo/topic
# 先修改Makefile中有关资源路径的配置
make
./topic_server
./publish_client
./subscribe_client
```

## 项目模块介绍

### 基础模块 (`base/`)

#### 核心抽象接口

- `base_buffer.h`：缓冲区基类，定义数据读写的统一接口
- `base_client.h`：客户端基类，提供连接、断开等基础功能接口  
- `base_connection.h`：连接基类，定义发送、关闭、状态检查等连接操作接口
- `base_message.h`：消息基类，所有消息类型的统一抽象
- `base_protocol.h`：协议基类，定义协议编解码的统一接口
- `base_server.h`：服务器基类，提供启动、停止等服务器基础功能

#### Muduo封装实现

- `muduo_buffer.h`：基于Muduo库Buffer的缓冲区实现
- `muduo_client.h`：基于Muduo TcpClient的客户端实现
- `muduo_connection.h`：基于Muduo TcpConnection的连接封装
- `muduo_server.h`：基于Muduo TcpServer的服务器实现

#### 消息处理

- `dispatcher.h`：消息分发器，根据消息类型调用对应的回调函数
- `json_message.h`：JSON格式消息的具体实现
- `request_message.h`：请求消息定义，包含RPC请求和主题操作请求
- `response_message.h`：响应消息定义，包含RPC响应和主题操作响应

#### 协议与数据

- `length_value_protocol.h`：长度+值协议实现，用于TCP粘包处理
- `public_data.h`：公共数据定义，包含消息类型、操作类型、错误码等枚举
- `log.h`：日志系统封装，基于spdlog实现

### 客户端模块 (`client/`)

- `main_client.h`：主要客户端类集合，包含服务注册、发现、RPC调用、主题等客户端
- `requestor.h`：请求发送器，处理请求的发送和响应接收
- `rpc_caller.h`：RPC调用器，封装具体的RPC方法调用逻辑
- `rpc_registry_client.h`：注册中心客户端，处理服务注册和发现
- `rpc_topic_client.h`：主题功能客户端实现

### 服务端模块 (`server/`)

- `main_server.h`：主要服务器类，包含RPC服务器、注册中心服务器、主题服务器
- `rpc_router.h`：RPC路由器，包含服务描述、服务管理器等
- `rpc_registry_server.h`：注册中心服务器实现
- `rpc_topic_server.h`：主题服务器实现，处理发布/订阅功能

### 工厂模块 (`factories/`)

- `buffer_factory.h`：缓冲区工厂，根据不同需求创建相应的缓冲区实例
- `connection_factory.h`：连接工厂，创建具体连接对象
- `message_factory.h`：消息工厂，根据不同需求创建不同的消息对象
- `protocol_factory.h`：协议工厂，创建具体协议对象
- `client_factory.h`：客户端工厂，创建具体客户端对象
- `server_factory.h`：服务端工厂，创建具体服务端对象

### 工具模块 (`utils/`)

- `JsonUtil.h`：JSON工具类，提供JSON序列化和反序列化功能，基于JSONCPP库实现
- `uuid_generator.h`：基于Boost的UUID生成工具类
