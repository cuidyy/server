cmake_minimum_required(VERSION 3.0)
project(server CXX)

# 定义变量
set(BUILD_PATH ${PROJECT_SOURCE_DIR})                       # 二进制文件输出路径
set(LINK_LIB event event_openssl ssl crypto mysqlclient jsoncpp spdlog fmt)       # 链接动态库列表 Dockerfile构建镜像时需要fmt，而本地编译不需要

# 生成可执行文件
# 搜索源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
# 包含头文件
include_directories(${PROJECT_SOURCE_DIR}/include)
# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${BUILD_PATH})
# 添加目标
add_executable(test ${SRC_LIST})

# 链接第三方库
# 链接库依赖文件

# 链接系统动态库
target_link_libraries(test ${LINK_LIB})




