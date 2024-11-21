# 编译器
CC = g++

# 编译选项
CFLAGS = -I/usr/include/jsoncpp/

# 目标可执行文件
TARGET = test

# 源文件列表
SRCS = main.cpp msgProcess.cpp Server.cpp base64_ssl.cpp requestProcess.cpp

# 头文件列表（如果有的话，可以根据实际情况添加）
# INCLUDES = -I/path/to/headers

# 库文件路径（如果需要指定特定路径，可以根据实际情况修改）
# LIBPATH = -L/path/to/libs

# 需要链接的库
LIBS = -levent -lssl -lcrypto -L/usr/local/lib -ljsoncpp
# 根据源文件列表生成目标文件列表
OBJS = $(SRCS:.cpp=.o)

# 目标规则：生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

# 清理规则：删除生成的目标文件和可执行文件
clean:
	rm -f $(TARGET)