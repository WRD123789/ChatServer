#include "groupmodel.hpp"
#include "database.hpp"

using namespace std;

bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO AllGroup(group_name, group_desc) VALUES('%s', '%s')", 
        group.getName().c_str(), group.getDesc().c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            // 获取插入成功后自动生成的群组 id
            group.setId(mysql_insert_id(mysql.getConnection()));

            return true;
        }
    }

    return false;
}

void GroupModel::addGroup(int userId, int groupId, string role)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO GroupUser VALUES(%d, %d, '%s')", 
        groupId, userId, role.c_str());
    
    MySQL mysql;
    if (mysql.connect())
        mysql.update(sql);
}

vector<Group> GroupModel::queryGroups(int userId)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT b.id, b.group_name, b.group_desc FROM GroupUser a \
        INNER JOIN AllGroup b ON a.group_id = b.id WHERE a.user_id = %d", 
        userId);
    
    vector<Group> groups;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groups.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    
    // 查询群组中用户的信息
    for (Group &group : groups) {
        sprintf(sql, "SELECT b.id, b.name, b.state, a.group_role FROM GroupUser a \
            INNER JOIN User b ON a.user_id = b.id WHERE a.group_id = %d", 
            group.getId());
        
        MySQL mysql;
        if (mysql.connect()) {
            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr) {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr) {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }
    }
    return groups;
}

vector<int> GroupModel::queryGroupUsers(int userId, int groupId)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT user_id FROM GroupUser WHERE group_id = %d \
        AND user_id <> %d", groupId, userId);
    
    vector<int> users;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
                users.push_back(atoi(row[0]));
            mysql_free_result(res);
        }
    }
    return users;
}