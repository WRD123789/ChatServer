#ifndef __DATABASE__
#define __DATABASE__

#include <mysql/mysql.h>
#include <string>

class MySQL {
public:
    MySQL();
    ~MySQL();

    bool connect();                    // 连接数据库
    bool update(std::string sql);      // 更新操作
    MYSQL_RES* query(std::string sql); // 查询操作
    inline MYSQL* getConnection() { return _conn; }

private:
    MYSQL *_conn;
};

#endif