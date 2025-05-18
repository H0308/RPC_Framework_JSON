CC=g++
CFLAGS=-std=c++17
INCLUDES=-I../../muduo_include -I/home/epsda/RPC_Framework_JSON/ -I/home/epsda/RPC_Framework_JSON/rpc_framework/utils/
LDFLAGS=-L../../muduo_lib -lmuduo_net -lmuduo_base -lpthread -lfmt -lspdlog -lboost_system -ljsoncpp -fsanitize=address -g

# 主要目标
all: registry_server rpc_client rpc_server

# 服务器可执行程序
registry_server:registry_server.cc
	$(CC) -o registry_server registry_server.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)
h
# 客户端可执行程序
rpc_client:rpc_client.cc
	$(CC) -o rpc_client rpc_client.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

rpc_server:rpc_server.cc
	$(CC) -o rpc_server rpc_server.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# 清理目标
.PHONY: clean
clean:
	rm -f registry_server rpc_client rpc_server
