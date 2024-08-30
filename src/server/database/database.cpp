#include "database.hpp"

#include <muduo/base/Logging.h>

using namespace std;

static string server = "127.0.0.1"; 
static string user = "test";
static string password = "123456"; 
static string dbName = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}

MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}

bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
        password.c_str(), dbName.c_str(), 3306, nullptr, 0);
    if (p != nullptr) {
        // 设置编码
        mysql_query(_conn, "set names utf8mb4");

        LOG_INFO << "数据库连接成功!";
    } else {
        LOG_INFO << "数据库连接失败!";
    }

    return p;
}

bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败!";
        return false;
    }
    
    return true;
}

MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败!";
        return nullptr;
    }
    
    return mysql_use_result(_conn);
}