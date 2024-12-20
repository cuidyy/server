# 编译器
CC = g++

# 编译选项
CFLAGS = -I./include -I/usr/include/jsoncpp/

# 目标可执行文件
TARGET = test

# 源文件列表
SRCS = $(wildcard ./src/*.cpp)

# 需要链接的库
LIBS = -levent -levent_openssl -lssl -lcrypto -lmysqlclient -L/usr/local/lib -ljsoncpp

# 目标规则：生成可执行文件
$(TARGET):
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# 清理规则：删除生成的可执行文件
clean:
	rm -f $(TARGET)