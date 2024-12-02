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
        Upload(user);
    }
    if(request == "getlist")
    {
        Json::Value user = root["user"];
        Getlist(user);
    }
    if(request == "download")
    {
        Json::Value user = root["user"];
        Download(user);
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
    //获取用户名，图片名，图片数据
    string username = user["username"].asString();
    string imagename = user["imagename"].asString();
    string imagedata = user["imagedata"].asString();

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();

    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        reply_msg["request"] = "upload";
        reply_msg["msg"] = "连接数据库失败";
        sendMsg();
        return;
    }

    string sql_str = "select * from user_img where username = \'"
                    + username + "\' and imagename = \'" + imagename + "\';";

    conn->query(sql_str);//查询数据库
    //遍历结果集，查看图片是否存在
    if(!conn->next())   //图片不存在，插入图片数据
    {
        string user_path = "/home/cuidy/test/picture/" + username;
        cout << user_path << endl;
        // 检测用户目录是否存在
        struct stat st;
        if(stat(user_path.c_str(), &st) == -1)
        {
            //如果用户目录不存在，创建用户目录
            if(mkdir(user_path.c_str(), 0755) == -1)
            {
                std::cerr << "创建用户目录失败" << endl;
                return;
            }
        }
        // 在用户目录下创建文件，并将数据写入文件
        // 图片路径
        string image_path = user_path + "/" + imagename;
        ofstream file(image_path, std::ios::out);
        if (!file) 
        {
            std::cerr << "创建文件失败" << endl;
            return;
        }
        //将imagedata写入文件
        file.write(imagedata.c_str(), imagedata.size());
        //关闭文件
        file.close();
        cout << "数据已成功写入文件" << endl;

        //将记录插入user_img表中
        string sql_insert = "insert into user_img (username, imagename, imagepath) values (\'"
                            + username + "\', \'" + imagename + "\', \'" + image_path + "\');";    
        
        if(!conn->update(sql_insert))
        {
            //插入数据库失败
            reply_msg["request"] = "upload";
            reply_msg["msg"] = "上传失败";
        }
        else
        {
            reply_msg["request"] = "upload";
            reply_msg["msg"] = "上传成功";
        }                   
    }
    else//图片存在不做任何处理直接回复
    {
        reply_msg["request"] = "upload";
        reply_msg["msg"] = "上传成功";
    }

    //释放结果集
    conn->freeResult();

    //回复消息
    sendMsg();
}

void requestProcess::Getlist(Json::Value user)
{
    string username = user["username"].asString();
    //图片名称列表
    Json::Value list;

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();

    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        reply_msg["request"] = "upload";
        reply_msg["msg"] = "连接数据库失败";
        sendMsg();
        return;
    }

    string sql_str = "select imagename from user_img where username = \'"
                    + username + "\';";

    conn->query(sql_str);//查询数据库
    while(conn->next())
    {
        string imgname = conn->value(0);
        list.append(imgname);
    }
    reply_msg["list"] = list;
    reply_msg["request"] = "getlist";
    
    //释放结果集
    conn->freeResult();
    
    sendMsg();
}

void requestProcess::Download(Json::Value user)
{
    string username = user["username"].asString();
    string imagename = user["imagename"].asString();

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();
    
    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        reply_msg["request"] = "download";
        reply_msg["msg"] = "连接数据库失败";
        sendMsg();
        return;
    }
    string sql_str = "select imagepath from user_img where username = \'"
                + username + "\' and imagename = \'" + imagename + "\';";

    conn->query(sql_str);//查询数据库
    //图片是否存在
    if(conn->next())//图片存在
    {
        string image_path = conn->value(0);
        ifstream file(image_path, std::ios::in);
        if (!file) 
        {
            std::cerr << "打开文件失败" << endl;
            return;
        }
        // 使用stringstream来存储读取到的数据
        std::stringstream buffer;
        buffer << file.rdbuf();
        // 获取存储在stringstream中的数据并转换为string类型
        std::string imagedata = buffer.str();
        //关闭文件
        file.close();
        cout << "数据已成功读出文件" << endl;

        reply_msg["imagedata"] = imagedata;
        reply_msg["msg"] = "下载成功";
        reply_msg["imagename"] = imagename;
        reply_msg["request"] = "download";
    }
    else
    {
        reply_msg["msg"] = "图片不存在";
        reply_msg["request"] = "download";
    }

    //释放结果集
    conn->freeResult();
    sendMsg();
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