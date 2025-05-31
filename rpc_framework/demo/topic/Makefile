CC=g++
CFLAGS=-std=c++17
INCLUDES=-I<Muduo头文件路径> -I<项目根路径> -I<项目根路径>/utils/
# 例如
# INCLUDES=-I../../muduo_include -I/home/epsda/RPC_Framework_JSON/ -I/home/epsda/RPC_Framework_JSON/rpc_framework/utils/
LDFLAGS=-L<Muduo库文件路径> -lmuduo_net -lmuduo_base -lpthread -lfmt -lspdlog -lboost_system -ljsoncpp

# 主要目标
all: topic_server publish_client subscribe_client

# 服务器可执行程序
topic_server: topic_server.cc
	$(CC) -o topic_server topic_server.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# 客户端可执行程序
publish_client: publish_client.cc
	$(CC) -o publish_client publish_client.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

subscribe_client: subscribe_client.cc
	$(CC) -o subscribe_client subscribe_client.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# 清理目标
.PHONY: clean
clean:
	rm -f topic_server publish_client subscribe_client
