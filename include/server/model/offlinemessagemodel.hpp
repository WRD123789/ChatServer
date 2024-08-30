#ifndef __OFFLINE_MESSAGE_MODEL__
#define __OFFLINE_MESSAGE_MODEL__

#include <string>
#include <vector>

class OfflineMessageModel {
public:
    // 存储用户离线消息
    void insert(int userId, std::string message);
    // 删除用户离线消息
    void remove(int userId);
    // 查询用户离线消息
    std::vector<std::string> query(int userId);
};

#endif