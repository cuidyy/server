# 编译器
CC = g++

# 编译选项
CFLAGS = -I/usr/include/jsoncpp/

# 目标可执行文件
TARGET = test

# 源文件列表
SRCS = main.cpp Server.cpp msgProcess.cpp requestProcess.cpp MysqlConn.cpp ConnPool.cpp base64_ssl.cpp

# 需要链接的库
LIBS = -levent -lssl -lcrypto -lmysqlclient -L/usr/local/lib -ljsoncpp

# 目标规则：生成可执行文件
$(TARGET):
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

# 清理规则：删除生成的可执行文件
clean:
	rm -f $(TARGET)