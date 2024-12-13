#include "Server.h"

Server::Server()
{
    // 初始化 SSL 库
    SSL_library_init();
    // 加载所有可用的 SSL 算法
    OpenSSL_add_all_algorithms();
    // 加载 SSL 错误字符串
    SSL_load_error_strings();
 
    // 创建新的 SSL 上下文，使用 TLS 服务器方法
    ctx = SSL_CTX_new(TLS_server_method());
    // 检查 SSL 上下文是否有效
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
 
    // 加载服务器证书和私钥
    if (!SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM)
        || !SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM)
        || !SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

Server::~Server()
{
    //释放SSL上下文
    SSL_CTX_free(ctx);

    //释放监听器资源
    evconnlistener_free(listener);

    //释放base
    event_base_free(base);

    spdlog::default_logger()->info("服务器关闭");
}

//运行
void Server::run()
{
    init();
    spdlog::default_logger()->info("服务器启动，监听端口：{}", ntohs(saddr.sin_port));
}

//服务器初始化
void Server::init()
{
    //初始化事件基础架构
    base = event_base_new();
    if(base == nullptr)
    {
        spdlog::default_logger()->error("libevent base 创建失败");
        exit(1);
    }

    //初始化服务器socket
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(8080);

    //初始化监听器对象
    listener = evconnlistener_new_bind(base, accept_cb, this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&saddr, sizeof(saddr));
    if(listener == nullptr)
    {
        spdlog::default_logger()->error("listener 创建失败");
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
    spdlog::default_logger()->info("ip:{} fd:{} 客户端连接 ", ip, fd);

    //获取服务器对象
    Server* server = (Server*)arg;   

    //创建SSL对象
    SSL *ssl = SSL_new(server->ctx);

    //获取base对象
    struct event_base *base = server->base;

    //创建bufferevent对象，与socket绑定，用于处理socket的读写、异常处理事件
    struct bufferevent *bev = bufferevent_openssl_socket_new(base, fd, ssl, BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
    if(bev)
    {
        spdlog::default_logger()->info("SSL/TLS 连接建立成功");
    }

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
    {
        spdlog::default_logger()->info("客户端断开连接");
        //释放bufferevent资源
        bufferevent_free(bev);
    }
    else if (events & BEV_EVENT_ERROR)
    {
        spdlog::default_logger()->info("客户端发生错误");
        bufferevent_free(bev);
    }
    else if (events & BEV_EVENT_TIMEOUT)
    {
        printf("超时\n");
        spdlog::default_logger()->info("超时");
        bufferevent_free(bev);
    }

}