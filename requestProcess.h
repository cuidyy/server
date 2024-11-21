#ifndef REQUESTPROCESS_H
#define REQUESTPROCESS_H
#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
using namespace std;

//请求处理
class requestProcess
{
public:
    requestProcess(struct bufferevent *bev);
    ~requestProcess();
    void Process(string msg);//根据request字段内容进行对应的处理

private:
    Login(Json::Value user);

    Register(Json::Value user);


private:
    Json::Value reply_msg;//用于回复的消息
    struct bufferevent *m_bev;//bufferevnt对象,用于回复消息
};

#endif