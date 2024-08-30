#ifndef __USER_MODEL__
#define __USER_MODEL__

#include "user.hpp"

class UserModel {
public:
    bool insert(User &user);
    User query(int id);          // 根据用户 id 查询用户信息
    bool updateState(User user); // 更新用户状态信息
    void resetState();           // 重置所有用户的状态信息
};

#endif