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
    if(!reader.parse(msg, root))
    {
        cout << "json解析失败" << endl;
    }
    cout << root << endl;
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
    if(request == "upload")
    {
        Json::Value user = root["user"];
        cout << user << endl;
        Upload(user);
    }
}

void requestProcess::Login(Json::Value user)
{
    //获取用户名和密码
    string username = user["username"].asString();
    string password = user["password"].asString();

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();

    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        reply_msg["request"] = "login";
        reply_msg["msg"] = "连接数据库失败";
        sendMsg();
        return;
    }

    string sql_str = "select * from user where username = \'"
                    + username + "\' and password = \'" + password + "\';";

    conn->query(sql_str);//查询数据库

    //遍历结果集，查看用户是否存在
    if(!conn->next())
    {
        reply_msg["request"] = "login";
        reply_msg["msg"] = "用户名不存在或密码有误";
        cout << "用户名不存在" << endl;
    }
    else
    {
        reply_msg["request"] = "login";
        reply_msg["msg"] = "登录成功";
        cout << "登录成功" << endl;
    }
    
    //释放结果集
    conn->freeResult();

    //回复消息
    sendMsg();
}

void requestProcess::Register(Json::Value user)
{
    //获取用户名和密码
    string username = user["username"].asString();
    string password = user["password"].asString();

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();

    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        reply_msg["request"] = "register";
        reply_msg["msg"] = "连接数据库失败";
        sendMsg();
        return;
    }

    string sql_str = "select * from user where username = \'"
                    + username + "\';";
                    
    conn->query(sql_str);//查询数据库
    //遍历结果集，查看用户是否存在
    if(!conn->next())   //用户不存在，插入新数据
    {
        string sql_insert = "insert into user (username, password) values (\'"
                            + username + "\', \'" + password + "\');";
        if(!conn->update(sql_insert))
        {
            //插入数据库失败
            reply_msg["request"] = "register";
            reply_msg["msg"] = "注册失败";
        }
        else
        {
            reply_msg["request"] = "register";
            reply_msg["msg"] = "注册成功";
            cout << "注册成功" << endl;
        }
    }
    else    //用户存在
    {
        reply_msg["request"] = "register";
        reply_msg["msg"] = "用户名已存在";
        cout << "用户名已存在" << endl;
    }

    //释放结果集
    conn->freeResult();

    //回复消息
    sendMsg();
}

void requestProcess::Upload(Json::Value user)
{
    
}

void requestProcess::sendMsg()
{
    Json::FastWriter writer;
    //将json格式转换为字符串
    string msg = writer.write(reply_msg);

    //base64编码
    msg = base64_encode(msg);

    //添加数据包头
    uint32_t size = msg.size();
    cout << size << endl;
    size = htonl(size);//转换为网络字节序

    //将数据写入缓存区
    bufferevent_write(m_bev, &size, 4);
    bufferevent_write(m_bev, msg.data(), msg.size());

    bufferevent_flush(m_bev, EV_WRITE, BEV_NORMAL);
}