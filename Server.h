#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>
#include <event.h>
#include <event2/buffer.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#include "msgProcess.h"
#include "requestProcess.h"
using namespace std;
#define BUFFER_SIZE 1024
class Server
{
public:
    Server();
    ~Server();
    void run();//运行
private:
    void init();//服务器初始化

    struct event_base * base = nullptr;//基础事件架构
    struct evconnlistener * listener = nullptr;//监听器
    struct sockaddr_in saddr;//服务器socket信息
    uint16_t port = 8080;
};

/**  
* @brief 读事件回调函数
* @param bev bufferevnet对象，与socket绑定，用于处理socket的读写、异常处理事件
* @param arg 传给回调函数的参数
*/
void read_cb(struct bufferevent *bev, void *arg);

/**  
* @brief accept回调函数 当有新连接建立时调用
* @param listen 监听器对象，完成了socket、bind、listen、accept全部操作
* @param fd 用于与客户端连接的socket
* @param addr 客户端的地址信息
* @param arg 传给回调函数的参数
*/
void accept_cb(struct evconnlistener *listen, evutil_socket_t fd, 
                    struct sockaddr * addr, int socklen, void *arg);

/**  
* @brief 异常事件回调函数
* @param bev bufferevnet对象，与socket绑定，用于处理socket的读写、异常处理事件
* @param events 发生的事件
* @param arg 传给回调函数的参数
*/
void event_cb(struct bufferevent *bev, short events, void *arg);


#endif