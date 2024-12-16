#ifndef MSGPROCESS_H
#define MSGPROCESS_H

#include <iostream>
#include <event.h>
#include <event2/buffer.h>
#include <time.h>
#include <stdlib.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <string>
#include "base64_ssl.h"
using namespace std;

class msgProcess
{
public:
    msgProcess(struct bufferevent *bev);
    ~msgProcess();
    bool recviece_msg();    //接收消息，从bufferevent的输入缓冲区中取出消息放入m_msg中
    void decode_msg(string msg);      //解密base64编码
    string get_msg();//返回解密后的数据
private:
    struct bufferevent *m_bev;  //bufferevent对象
    struct evbuffer *m_evbuf;   //bufferevent对象的输入缓冲区
    string m_msg;               //解密后的消息数据
};

#endif