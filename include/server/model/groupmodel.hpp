#ifndef __GROUP_MODEL__
#define __GROUP_MODEL__

#include "group.hpp"

class GroupModel {
public:
    // 创建群组
    bool createGroup(Group &group);
    // 加入群组
    void addGroup(int userId, int groupId, std::string role);
    // 查询用户所在的所有群组信息
    std::vector<Group> queryGroups(int userId);
    // 查询指定群组中除 userId 外的其他用户的 id, 用于群发信息
    std::vector<int> queryGroupUsers(int userId, int groupId);
};

#endif