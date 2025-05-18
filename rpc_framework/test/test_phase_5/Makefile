CC=g++
CFLAGS=-std=c++17
INCLUDES=-I../../muduo_include -I/home/epsda/RPC_Framework_JSON/ -I/home/epsda/RPC_Framework_JSON/rpc_framework/utils/
LDFLAGS=-L../../muduo_lib -lmuduo_net -lmuduo_base -lpthread -lfmt -lspdlog -lboost_system -ljsoncpp -fsanitize=address -g

# 主要目标
all: server client

# 服务器可执行程序
server:server.cc
	$(CC) -o server server.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# 客户端可执行程序
client:client.cc
	$(CC) -o client client.cc $(CFLAGS) $(INCLUDES) $(LDFLAGS)

# 清理目标
.PHONY: clean
clean:
	rm -f server client
