#pragma once
#include <mysql/mysql.h>
#include <string>
#include <chrono>
#include <spdlog/spdlog.h>
using namespace std;
using namespace std::chrono;

//Mysql操作类
class MysqlConn {
public:
    // 初始化数据库连接
    MysqlConn();
    // 释放数据库连接
    ~MysqlConn();
    // 连接数据库
    bool connect(string user, string passwd, string dbName, string ip, unsigned short port = 3306);
    // 更新数据库: select，update，delete
    bool update(string sql);
    // 查询数据库
    bool query(string sql);
    // 遍历查询得到的结果集
    bool next();
    // 得到结果集中的字段值
    string value(int index);
    // 事务操作
    bool transaction();
    // 提交事务
    bool commit();
    // 事务回滚
    bool rollback();
    // 刷新起始的空闲时间点
    void refreshAliveTime();
    // 计算连接存活的总时长
    long long getAliveTime();

    void freeResult();//释放结果集

private:
    
    MYSQL* m_conn = nullptr; // 数据库连接
    MYSQL_RES* m_result = nullptr;//结果集
    MYSQL_ROW m_row = nullptr;//行数据
    steady_clock::time_point m_aliveTime;//数据库连接建立的起始时间
};
