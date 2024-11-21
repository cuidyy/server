#include "msgProcess.h"

msgProcess::msgProcess(struct bufferevent *bev) : m_bev(bev)
{

}

msgProcess::~msgProcess()
{

}

bool msgProcess::recviece_msg()
{
    //获取输入缓冲区
    m_evbuf = bufferevent_get_input(m_bev);

    //获取当前缓存区存放数据大小
    int buf_len = evbuffer_get_length(m_evbuf);

    //数据表头不完整
    if(buf_len < 4)
    {
        cout << "数据不完整"<< endl;
        return false;
    }

    //需要读取的数据长度
    int data_len = 0;
    //读取数据包表头，提取数据包长度
    bufferevent_read(m_bev, (char*)&data_len, 4);
    data_len = ntohl(data_len);

    char buffer[4096];//接收缓冲区
    string msg;//接收缓冲区的数据
    //循环接收数据
    while(data_len > 0)
    {
        int size = (data_len <= 4096) ? data_len : 4096;
        bufferevent_read(m_bev, buffer, size);//读取数据
        msg.append(buffer, size);
        data_len -= size;
    }
    decode_msg(msg);
    return true;
}

//base64编码解密
void msgProcess::decode_msg(string msg)
{
    m_msg = base64_decode(msg);
}

string msgProcess::get_msg()
{
    return m_msg;
}