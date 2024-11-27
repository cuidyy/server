#include "msgProcess.h"
#include <thread>
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
        return false;
    }

    //需要读取的数据长度
    uint32_t data_len = 0;
    //读取数据包表头，提取数据包长度
    //将缓冲区前4字节数据拷贝到data_len
    evbuffer_copyout(m_evbuf, &data_len, 4);
    data_len = ntohl(data_len);
    if(buf_len < 4 + data_len)//缓冲区还没有接受到所有数据，直接返回，直到所有数据到达
    {
        return false;
    }

    //接受完整消息
    string msg;
    msg.resize(data_len);
    //丢弃包头
    evbuffer_drain(m_evbuf, 4);
    //提取消息
    evbuffer_remove(m_evbuf, &msg[0], data_len);
    //base64解码
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