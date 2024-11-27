#include "Server.h"

Server::Server()
{

}

Server::~Server()
{
    //释放监听器资源
    evconnlistener_free(listener);

    //释放base
    event_base_free(base);
}

//运行
void Server::run()
{
    init();
}

//服务器初始化
void Server::init()
{
    //初始化事件基础架构
    base = event_base_new();
    if(base == nullptr)
    {
        cout << "base 创建失败" << endl;
        exit(1);
    }

    //初始化服务器socket
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(8080);

    //初始化监听器对象
    listener = evconnlistener_new_bind(base, accept_cb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&saddr, sizeof(saddr));
    if(listener == nullptr)
    {
        cout << "listener 创建失败" << endl;
        exit(1);
    }

    //开启事件主循环
    event_base_dispatch(base);
}

void accept_cb(struct evconnlistener *listen, evutil_socket_t fd, 
                    struct sockaddr * addr, int socklen, void *arg)
{
    char ip[32] = {0};
    evutil_inet_ntop(AF_INET, addr, ip, sizeof(ip)-1);
    printf("accept a client fd:%d ip:%s\n",fd,ip);

    struct event_base *base = (struct event_base *)arg;    //把base传进来了

    //构建bufferevent对象 对fd创建bufferevent事件
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    //设置读回调函数、异常处理函数
    bufferevent_setcb(bev, read_cb, NULL, event_cb, NULL);
    bufferevent_enable(bev,EV_READ | EV_PERSIST);   //注册事件
}

//读事件回调函数
void read_cb(struct bufferevent *bev, void *arg)
{
    msgProcess mpc(bev);//接收并处理数据
    if(!mpc.recviece_msg())//数据不完整
    {
        return;
    }
    
    string msg = mpc.get_msg();//获取解密后的数据
    requestProcess rpc(bev);
    rpc.Process(msg);//处理请求
}

//异常事件回调函数
void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if(events & BEV_EVENT_EOF)//与客户端断开连接
        printf("客户端断开连接\n");
    else if (events & BEV_EVENT_ERROR)
        printf("发送错误\n");
    else if (events & BEV_EVENT_TIMEOUT)
        printf("超时\n");

    //释放bufferevent资源
    bufferevent_free(bev);
}