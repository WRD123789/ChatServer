#include "usermodel.hpp"
#include "database.hpp"

#include <iostream>

using namespace std;

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO User(name, password, state) VALUES('%s', '%s', '%s')", 
        user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            // 获取插入成功后自动生成的用户 id
            user.setId(mysql_insert_id(mysql.getConnection()));

            return true;
        }
    }

    return false;
}

User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT * FROM User WHERE id = %d", id);
    
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr) {
                User user(atoi(row[0]), row[1], row[2], row[3]);

                mysql_free_result(res);

                return user;
            }
        }
    }

    return User();
}

bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "UPDATE User SET state = '%s' WHERE id = %d", 
        user.getState().c_str(), user.getId());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql))
            return true;
    }

    return false;
}

void UserModel::resetState()
{
    char sql[1024] = {0};
    sprintf(sql, "UPDATE User SET state = 'offline' WHERE state = 'online'");
    
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}