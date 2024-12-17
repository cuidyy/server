# 基础镜像，选用Ubuntu作为基础镜像，你也可以根据实际情况选择其他合适的基础镜像
FROM ubuntu:22.04

# 更新软件包列表并安装必要的软件和库，包括编译工具、项目依赖的相关库以及其头文件等
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    cmake \
    libevent-dev \
    libevent-openssl-2.1-7 \
    libssl-dev \
    libcrypto++-dev \
    libmysqlclient-dev \
    libjsoncpp-dev \
    libspdlog-dev || { echo "APT install failed"; exit 1; }
    # 根据实际情况，如有其他缺失的依赖库，在此处添加对应的安装命令

# 以下是可选的优化步骤，用于清理不再需要的安装包和缓存文件，减小镜像体积
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*
    
# 创建用于存储图片的目录以及服务端项目所在目录，对应宿主机的相关路径
RUN mkdir -p /server /picture

# 将宿主机上的服务端项目文件复制到镜像内的/home/cuidy/test/server/目录，会根据.dockerignore文件规则忽略相应文件
COPY ./ /server/

# 进入服务端项目所在的工作目录
WORKDIR /server

# 使用 CMake 构建项目，配置、生成并编译项目
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

# 暴露服务端监听的端口，例如8888，你可替换为实际使用的端口号
EXPOSE 8080

# 设置容器启动时的命令，运行服务端可执行文件，此处假设可执行文件名为'test'，可根据实际替换
CMD ["./test"]

