#include "friendmodel.hpp"
#include "database.hpp"

using namespace std;

void FriendModel::insert(int userId, int friendId)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO Friend VALUES(%d, %d)", userId, friendId);
    
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}

vector<User> FriendModel::query(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.name, a.state FROM User a INNER JOIN Friend b \
        ON a.id = b.friend_id WHERE b.user_id = %d", userId);
    
    vector<User> friends;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friends.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return friends;
}