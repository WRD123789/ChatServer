#ifndef __FRIEND_MODEL__
#define __FRIEND_MODEL__

#include "user.hpp"

#include <vector>

class FriendModel {
public:
    void insert(int userId, int friendId); // 添加好友关系
    std::vector<User> query(int userId);   // 查询好友列表
};

#endif