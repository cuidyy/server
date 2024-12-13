#include "requestProcess.h"

requestProcess::requestProcess(struct bufferevent *bev) :m_bev(bev)
{

}

requestProcess::~requestProcess()
{

}

void requestProcess::Process(string msg)
{
    //解析http请求
    std::istringstream iss(msg);

    // 解析请求方法
    std::string method, reqpath;//请求方法 请求路径
    std::getline(iss, method, ' ');

    //解析请求路径
    std::getline(iss, reqpath,' ');
    // 提取请求路径中的参数
    std::string request_params;
    int pos = reqpath.find('?');    //请求路径与参数的分隔符
    if (pos!= std::string::npos)
    {
        request_params = reqpath.substr(pos + 1);//获取请求参数
        reqpath = reqpath.substr(0, pos);//获取请求路径
    }

    //解析请求参数
    std::unordered_map<std::string, std::string> params;
    std::istringstream param_stream(request_params);
    std::string param;
    while (getline(param_stream, param, '&'))
    {
        int equal_pos = param.find('=');
        if (equal_pos != std::string::npos)
        {
            std::string key = param.substr(0, equal_pos);
            std::string value = param.substr(equal_pos + 1);
            params[key] = value;
        }
    }

    if(method == "POST")
    {
        //提取请求体
        std::string request_body;
        int pos = msg.find("\r\n\r\n");
        request_body = msg.substr(pos + 4);

        //解析请求体内容
        Json::Reader reader;
        Json::Value root;
        //reader将Json字符串解析到root，root将包含Json里面所有的子元素
        if(!reader.parse(request_body, root))
        {
            spdlog::default_logger()->error("json解析失败");
            return;
        }

        //根据请求类型进行处理
        if(reqpath == "/login")
        {
            Json::Value user = root["user"];
            spdlog::default_logger()->info("Login request");
            Login(user);
        }
        if(reqpath == "/register")
        {
            spdlog::default_logger()->info("Register request");
            Json::Value user = root["user"];
            Register(user);
        }
        if(reqpath == "/upload")
        {
            spdlog::default_logger()->info("Upload request");
            Json::Value user = root["user"];
            Upload(user);
        }

    }
    else if(method == "GET")
    {
        //根据请求类型进行处理
        if(reqpath == "/getlist")
        {
            spdlog::default_logger()->info("Getlist request");
            Json::Value user;
            user["username"] = params["username"];
            Getlist(user);
        }
        if(reqpath == "/download")
        {
            spdlog::default_logger()->info("Download request");
            Json::Value user;
            user["username"] = params["username"];
            user["imagename"] = params["imagename"];
            Download(user);
        }   
    }
    else if(method == "DELETE")
    {
        //根据请求类型进行处理
        if(reqpath == "/delete")
        {
            spdlog::default_logger()->info("Delete request");
            Json::Value user;
            user["username"] = params["username"];
            user["imagename"] = params["imagename"];
            Delete(user);
        }
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
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "login";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        sendMsg();
        return;
    }

    string sql_str = "select * from user where username = \'"
                    + username + "\' and password = \'" + password + "\';";

    conn->query(sql_str);//查询数据库

    //遍历结果集，查看用户是否存在
    // 用户不存在
    if(!conn->next())
    {
        spdlog::default_logger()->info("用户名不存在或密码有误");
        reply_msg["request"] = "login";
        reply_msg["msg"] = "用户名不存在或密码有误";
        status_line = "HTTP/1.1 403 Bad Request\r\n\r\n";
    }
    else    // 用户存在
    {
        spdlog::default_logger()->info("用户 {} 登录成功", username);
        reply_msg["request"] = "login";
        reply_msg["msg"] = "登录成功";
        status_line = "HTTP/1.1 200 OK\r\n\r\n";
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
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "register";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
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
            spdlog::default_logger()->info("用户 {} 注册失败", username);
            reply_msg["request"] = "register";
            reply_msg["msg"] = "注册失败";
            status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        }
        else
        {
            spdlog::default_logger()->info("用户 {} 注册成功", username);
            reply_msg["request"] = "register";
            reply_msg["msg"] = "注册成功";
            status_line = "HTTP/1.1 201 Created\r\n\r\n";
        }
    }
    else    //用户存在
    {
        spdlog::default_logger()->info("用户名 {} 已存在", username);
        reply_msg["request"] = "register";
        reply_msg["msg"] = "用户名已存在";
        status_line = "HTTP/1.1 403 Bad Request\r\n\r\n";
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
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "upload";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
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
        // 检测用户目录是否存在
        struct stat st;
        if(stat(user_path.c_str(), &st) == -1)
        {
            //如果用户目录不存在，创建用户目录
            if(mkdir(user_path.c_str(), 0755) == -1)
            {
                spdlog::default_logger()->error("创建用户目录失败");
                return;
            }
        }
        // 在用户目录下创建文件，并将数据写入文件
        // 图片路径
        string image_path = user_path + "/" + imagename;
        ofstream file(image_path, std::ios::out);
        if (!file) 
        {
            spdlog::default_logger()->error("创建文件失败");
            return;
        }
        //将imagedata写入文件
        file.write(imagedata.c_str(), imagedata.size());
        //关闭文件
        file.close();

        spdlog::default_logger()->info("数据成功写入文件");

        //将记录插入user_img表中
        string sql_insert = "insert into user_img (username, imagename, imagepath) values (\'"
                            + username + "\', \'" + imagename + "\', \'" + image_path + "\');";    
        
        if(!conn->update(sql_insert))
        {
            //插入数据库失败
            spdlog::default_logger()->info("图片 {} 上传失败", imagename);
            reply_msg["request"] = "upload";
            reply_msg["msg"] = imagename + "上传失败";
            status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        }
        else
        {
            spdlog::default_logger()->info("图片 {} 上传成功", imagename);
            reply_msg["request"] = "upload";
            reply_msg["msg"] = imagename + "上传成功";
            status_line = "HTTP/1.1 200 OK\r\n\r\n";
        }                   
    }
    else//图片存在不做任何处理直接回复
    {
        spdlog::default_logger()->info("图片 {} 已存在", imagename);
        reply_msg["request"] = "upload";
        reply_msg["msg"] = imagename + "上传成功";
        status_line = "HTTP/1.1 200 OK\r\n\r\n";
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
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "upload";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
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
    status_line = "HTTP/1.1 200 OK\r\n\r\n";
    spdlog::default_logger()->info("获取列表成功");
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
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "download";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
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
            spdlog::default_logger()->error("文件 {} 打开失败", imagename);
            return;
        }
        // 使用stringstream来存储读取到的数据
        std::stringstream buffer;
        buffer << file.rdbuf();
        // 获取存储在stringstream中的数据并转换为string类型
        std::string imagedata = buffer.str();
        //关闭文件
        file.close();
        spdlog::default_logger()->info("文件 {} 读取成功", imagename);

        reply_msg["imagedata"] = imagedata;
        reply_msg["msg"] = "下载成功";
        reply_msg["imagename"] = imagename;
        reply_msg["request"] = "download";
        status_line = "HTTP/1.1 200 OK\r\n\r\n";
        spdlog::default_logger()->info("下载成功");
    }
    else
    {
        reply_msg["msg"] = "图片不存在";
        reply_msg["request"] = "download";
        status_line = "HTTP/1.1 403 Bad Request\r\n\r\n";
        spdlog::default_logger()->info("图片 {} 不存在", imagename);
    }

    //释放结果集
    conn->freeResult();
    sendMsg();
}

void requestProcess::Delete(Json::Value user)
{
    string username = user["username"].asString();
    string imagename = user["imagename"].asString();

    //获取数据库连接
    auto conn = ConnPool::getConnPool()->getConn();
    //数据库连接失败,直接返回
    if(!conn->is_connected())
    {
        spdlog::default_logger()->error("连接数据库失败");
        reply_msg["request"] = "delete";
        reply_msg["msg"] = "连接数据库失败";
        status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        sendMsg();
        return;
    }
    string sql_str = "select * from user_img where username = \'"
                    + username + "\' and imagename = \'" + imagename + "\';";

    conn->query(sql_str);//查询数据库
    //图片是否存在
    if(conn->next())//图片存在
    {
        string sql_delete = "delete from user_img where username = \'"
                            + username + "\' and imagename = \'" + imagename + "\';";
        if(!conn->update(sql_delete))   
        {
            spdlog::default_logger()->info("删除失败");
            reply_msg["request"] = "delete";
            reply_msg["msg"] = "删除失败";
            status_line = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
        }
        else//删除成功返回新的图片列表
        {
            //图片名称列表
            Json::Value list;
            string sql_str = "select imagename from user_img where username = \'"
                    + username + "\';";

            conn->query(sql_str);//查询数据库
            while(conn->next())
            {
                string imgname = conn->value(0);
                list.append(imgname);
            }
            reply_msg["request"] = "delete";
            reply_msg["msg"] = "删除成功";
            reply_msg["list"] = list;
            status_line = "HTTP/1.1 200 OK\r\n\r\n";
            spdlog::default_logger()->info("删除成功");
        }
    }
    else//图片不存在
    {
        reply_msg["request"] = "delete";
        reply_msg["msg"] = "图片不存在";
        status_line = "HTTP/1.1 403 Bad Request\r\n\r\n";
        spdlog::default_logger()->info("图片 {} 不存在", imagename);
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

    //添加状态行
    msg = status_line + msg;
    
    //base64编码
    msg = base64_encode(msg);

    //添加数据包头
    uint32_t size = msg.size();
    size = htonl(size);//转换为网络字节序

    //将数据写入缓存区
    bufferevent_write(m_bev, &size, 4);
    bufferevent_write(m_bev, msg.data(), msg.size());

    bufferevent_flush(m_bev, EV_WRITE, BEV_NORMAL);
}