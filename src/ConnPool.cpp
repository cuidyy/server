#include "ConnPool.h"
#include <fstream>
#include <thread>
#include <iostream>
using namespace Json;

//获得单例
ConnPool* ConnPool::getConnPool() {
    static ConnPool pool;
    return &pool;
}
 
// 从连接池中取出一个连接
shared_ptr<MysqlConn> ConnPool::getConn() {
    unique_lock<mutex> locker(m_mutexQ);
    while (m_connQ.empty()) {//连接队列为空
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout))) {
            if (m_connQ.empty()) {
                //return nullptr;
                continue;
            }
        }
    }
    shared_ptr<MysqlConn>connptr(m_connQ.front(), [this](MysqlConn* conn) {
        lock_guard<mutex>locker(m_mutexQ); // 自动管理加锁和解锁
        conn->refreshAliveTime();// 更新连接的起始的空闲时间点
        m_connQ.push(conn); // 回收数据库连接，此时它再次处于空闲状态
        });// 智能指针
    m_connQ.pop();
    m_cond.notify_one(); // 本意是唤醒生产者
    return connptr;
}
 
ConnPool::~ConnPool() {
    while (!m_connQ.empty()) {
        MysqlConn* conn = m_connQ.front();
        m_connQ.pop();
        delete conn;
    }
}

ConnPool::ConnPool() {
    // 加载配置文件
    if (!parseJsonFile()) {
        spdlog::default_logger()->error("加载配置文件失败!!!");
        return;
    }
    spdlog::default_logger()->info("配置文件加载成功...");
    for (int i = 0; i < m_minSize; ++i) {
        if(!addConn())
        {
            spdlog::default_logger()->error("添加连接失败!!!");
            return;
        }
    }
    thread producer(&ConnPool::produceConn, this);// 生产连接
    thread recycler(&ConnPool::recycleConn, this);// 销毁连接
    producer.detach();
    recycler.detach();
}

//解析文件
bool ConnPool::parseJsonFile() {
    ifstream ifs("mysql.json");
    Json::Reader rd;
    Json::Value root;
    rd.parse(ifs, root);
    if (root.isObject()) {
        spdlog::default_logger()->info("开始解析配置文件...");
        m_ip = root["ip"].asString();
        m_port = root["port"].asInt();
        m_user = root["userName"].asString();
        m_passwd = root["password"].asString();
        m_dbName = root["dbName"].asString();
        m_minSize = root["minSize"].asInt();
        m_maxSize = root["maxSize"].asInt();
        m_maxIdleTime = root["maxIdleTime"].asInt();
        m_timeout = root["timeout"].asInt();
        return true;  // 解析成功返回true，否则返回false。
    }
    return false;
}

//生产连接线程
void ConnPool::produceConn() {
    while (true) {  // 生产者线程不断生产连接，直到连接池达到最大值
        unique_lock<mutex> locker(m_mutexQ);  // 加锁，保证线程安全
        while (m_connQ.size() >= m_minSize) {
            m_cond.wait(locker);  // 等待消费者通知
        }
        if (is_ConnPoolClosed) //连接池关闭，线程退出不再生产新的连接
        {
            return;
        }
        addConn(); // 生产连接
        m_cond.notify_all();// 通知消费者(唤醒)
    }
}
 
// 回收连接线程
void ConnPool::recycleConn() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(500));// 每隔半秒钟检测一次
        lock_guard<mutex> locker(m_mutexQ);  // 加锁，保证线程安全
        while (m_connQ.size() > m_minSize) {  // 如果连接池中的连接数大于最小连接数，则回收连接
            MysqlConn* conn = m_connQ.front();  // 取出连接池中的连接
            if (conn->getAliveTime() >= m_maxIdleTime) {
                m_connQ.pop();  // 回收连接
                delete conn;  // 释放连接资源
            }
            else {
                break;  // 如果连接的空闲时间小于最大空闲时间，则跳出循环
            }
        }
    }
}
 
// 添加连接到连接池
bool ConnPool::addConn() {
    MysqlConn* conn = new MysqlConn;
    if(!conn->connect(m_user, m_passwd, m_dbName, m_ip, m_port))
    {
        is_ConnPoolClosed = true;    // 连接失败，连接池关闭
        return false;
    }
    conn->refreshAliveTime();// 记录建立连接的时候的对应的时间戳
    m_connQ.push(conn);
    return true;
}

// 判断连接池是否关闭
bool ConnPool::isConnPoolClosed(){
    return is_ConnPoolClosed;
}