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
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

//请求处理
class requestProcess
{
public:
    requestProcess(struct bufferevent *bev);
    ~requestProcess();
    void Process(string msg);//根据request字段内容进行对应的处理

private:
    void Login(Json::Value user);//登录请求处理

    void Register(Json::Value user);//注册请求处理

    void Upload(Json::Value user);//图片上传请求处理

    void Getlist(Json::Value user);//获取用户图片列表请求处理

    void Download(Json::Value user);//用户下载图片处理

    void Delete(Json::Value user);//删除图片处理
    
    void sendMsg();//发送消息给客户端
private:
    std::string staus_line;//状态行
    Json::Value reply_msg;//用于回复的消息 响应体
    struct bufferevent *m_bev;//bufferevnt对象,用于回复消息
};

#endif