#include "chatservice.hpp"
#include "public.hpp"

#include <muduo/base/Logging.h>
#include <string>
#include <vector>

using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的消息处理函数
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, 
        bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, 
        bind(&ChatService::regis, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, 
        bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, 
        bind(&ChatService::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, 
        bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, 
        bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, 
        bind(&ChatService::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, 
        bind(&ChatService::loginout, this, _1, _2, _3)});
    
    if (_redis.connect())
        _redis.initNotifyHandler(bind(&ChatService::handleRedisSubscribeMessage,
            this, _1, _2));
}

void ChatService::login(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPassword() == password) {
        if (user.getState() == "online") {
            // 重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "This account is using, input another!";
            conn->send(response.dump());
        } else {
            // 登录成功
            // 记录用户连接信息
            unique_lock<mutex> lock(_connMutex);
            _userConnMap.insert({id, conn});
            lock.unlock();

            // 用户登录成功后, 向 Redis 订阅 channel(userid)
            _redis.subscribe(id);

            // 更新用户登录状态
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = id;
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> msg = _offlineMessageModel.query(id);
            if (!msg.empty()) {
                response["offlinemsg"] = msg;

                // 删除离线消息
                _offlineMessageModel.remove(id);
            }

            // 查询用户的好友信息并返回
            vector<User> friends = _friendModel.query(id);
            if (!friends.empty()) {
                vector<string> tmp;
                for (User &user : friends) {
                    json tmpJs;
                    tmpJs["id"] = user.getId();
                    tmpJs["name"] = user.getName();
                    tmpJs["state"] = user.getState();
                    tmp.push_back(tmpJs.dump());
                }
                response["friends"] = tmp;
            }

            // 查询用户的群组信息并返回
            vector<Group> groups = _groupModel.queryGroups(id);
            if (!groups.empty()) {
                vector<string> tmp;
                for (Group &group : groups) {
                    json tmpJs;
                    tmpJs["id"] = group.getId();
                    tmpJs["groupname"] = group.getName();
                    tmpJs["groupdesc"] = group.getDesc();

                    vector<GroupUser> users = group.getUsers();
                    if (!users.empty()) {
                        vector<string> tmp0;
                        for (GroupUser user : users) {
                            json tmpJs0;
                            tmpJs0["id"] = user.getId();
                            tmpJs0["name"] = user.getName();
                            tmpJs0["state"] = user.getState();
                            tmpJs0["role"] = user.getRole();
                            tmp0.push_back(tmpJs0.dump());
                        }
                        tmpJs["users"] = tmp0;
                    }
                    
                    tmp.push_back(tmpJs.dump());
                }
                response["groups"] = tmp;
            }

            conn->send(response.dump());
        }
    } else {
        // 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "Password or id is invalid!";
        conn->send(response.dump());
    }
}

void ChatService::regis(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);
    // 响应消息
    if (state) {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int toId = js["toid"].get<int>();

    unique_lock<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toId);
    if (it != _userConnMap.end()) {
        // toId 在线, 转发消息
        it->second->send(js.dump());
        return; // 析构自动释放锁
    }
    lock.unlock();

    // 判断 toid 用户是否在线
    User user = _userModel.query(toId);
    if (user.getState() == "online") {
        // 发布消息给对应的 Redis 通道
        _redis.publish(toId, js.dump());
        return;
    }

    // toId 不在线, 存储离线消息
    _offlineMessageModel.insert(toId, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int userId = js["id"];
    int friendId = js["friendid"];

    _friendModel.insert(userId, friendId);
}

void ChatService::createGroup(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    string groupName = js["groupname"];
    string groupDesc = js["groupdesc"];

    Group group(-1, groupName, groupDesc);
    if (_groupModel.createGroup(group))
        _groupModel.addGroup(userId, group.getId(), "creator");
}

void ChatService::addGroup(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();
    _groupModel.addGroup(userId, groupId, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();
    
    vector<int> groupMembers = _groupModel.queryGroupUsers(userId, groupId);
    unique_lock<mutex> lock(_connMutex);
    for (int member : groupMembers) {
        auto it = _userConnMap.find(member);
        if (it != _userConnMap.end()) {
            // 转发群消息
            it->second->send(js.dump());
        } else {
            // 判断 member 用户是否在线
            User user = _userModel.query(member);
            if (user.getState() == "online") {
                // 发布消息给对应的 Redis 通道
                _redis.publish(member, js.dump());
                return;
            }

            // 存储离线消息
            _offlineMessageModel.insert(member, js.dump());
        }
    }
}

void ChatService::loginout(const TcpConnectionPtr &conn,
    json &js, Timestamp time)
{
    int userId = js["id"].get<int>();

    unique_lock<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userId);
    if (it != _userConnMap.end())
        _userConnMap.erase(it);
    lock.unlock();

    // 在 Redis 中取消订阅通道
    _redis.unsubscribe(userId);

    // 更新用户登录状态
    User user(userId);
    _userModel.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    unique_lock<mutex> lock(_connMutex);
    for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it) {
        if (it->second == conn) {
            user.setId(it->first);

            // 从表中删除用户的连接信息
            _userConnMap.erase(it);
            break;
        }
    }
    lock.unlock();

    // 在 Redis 中取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新用户登录状态
    if (user.getId() != -1) {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end()) {
        return [=](const TcpConnectionPtr, json&, Timestamp) { 
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    } else {
        return _msgHandlerMap[msgid];
    }
}

void ChatService::reset()
{
    // 把所有用户的状态信息设置为 offline
    _userModel.resetState();
}

void ChatService::handleRedisSubscribeMessage(int userId, std::string msg)
{
    unique_lock<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userId);
    if (it != _userConnMap.end()) {
        it->second->send(msg);
        return;
    }
    lock.unlock();

    // 存储为离线消息
    _offlineMessageModel.insert(userId, msg);
}