#ifndef REQUESTPROCESS_H
#define REQUESTPROCESS_H
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include <stdlib.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include "base64_ssl.h"
#include "ConnPool.h"
#include "MysqlConn.h"
using namespace std;

//请求处理
class requestProcess
{
public:
    requestProcess(struct bufferevent *bev);
    ~requestProcess();
    void Process(string msg);//根据request字段内容进行对应的处理

private:
    void Login(Json::Value user);

    void Register(Json::Value user);

    void sendMsg();//发送消息给客户端
private:
    Json::Value reply_msg;//用于回复的消息
    struct bufferevent *m_bev;//bufferevnt对象,用于回复消息
};

#endif