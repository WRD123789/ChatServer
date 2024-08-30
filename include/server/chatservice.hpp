#ifndef __CHAT_SERVICE__
#define __CHAT_SERVICE__

#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

using json = nlohmann::json;
using MsgHandler = std::function<void (const muduo::net::TcpConnectionPtr &conn,
    json &js, muduo::Timestamp time)>;

// 聊天服务器业务类
class ChatService {
public:
    // 获取单例对象的接口函数
    static ChatService* instance();

    // 处理登录业务
    void login(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 处理注册业务
    void regis(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 一对一聊天业务
    void oneChat(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 添加好友业务
    void addFriend(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 创建群组业务
    void createGroup(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 加入群组业务
    void addGroup(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 群组聊天业务
    void groupChat(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    // 退出登录业务
    void loginout(const muduo::net::TcpConnectionPtr &conn,
        json &js, muduo::Timestamp time);
    
    // 处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr &conn);

    // 获取消息对应的处理函数
    MsgHandler getHandler(int msgid);
    // 服务器异常退出时, 用于业务信息重置
    void reset();

    // 从 Redis 消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userId, std::string msg);

private:
    ChatService();

    // 存储消息 id 和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接
    std::unordered_map<int, muduo::net::TcpConnectionPtr> _userConnMap;
    // 保证 _userConnMap 的线程安全
    std::mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMessageModel _offlineMessageModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // Redis 对象
    Redis _redis;
};

#endif