#include "requestProcess.h"

requestProcess::requestProcess(struct bufferevent *bev) :m_bev(bev)
{

}

requestProcess::~requestProcess()
{

}

void requestProcess::Process(string msg)
{
    Json::Reader reader;
    Json::Value root;
    //reader将Json字符串解析到root，root将包含Json里面所有的子元素
    if(!reader.parse(msg, str))
    {
        cout << "json解析失败" << endl;
    }
    string request = root["request"].asString();//请求内容

    if(request == "login")
    {
        Json::Value user = root["user"];
        Login(user);
    }
    if(request == "register")
    {
        Json::Value user = root["user"];
        Register(user);
    }
}

void requestProcess::Login(Json::Value user)
{

}

void requestProcess::Register(Json::Value user)
{

}