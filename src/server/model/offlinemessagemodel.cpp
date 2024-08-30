#include "offlinemessagemodel.hpp"
#include "database.hpp"

using namespace std;

void OfflineMessageModel::insert(int userId, string message)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO OfflineMessage VALUES(%d, '%s')", 
        userId, message.c_str());
    
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}

void OfflineMessageModel::remove(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "DELETE FROM OfflineMessage WHERE user_id = %d", userId);
    
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}

vector<string> OfflineMessageModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT message FROM OfflineMessage WHERE user_id = %d", userId);
    
    vector<string> msg;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
                msg.push_back(row[0]);
            mysql_free_result(res);
        }
    }
    return msg;
}