#include "redis.hpp"

#include <iostream>

using namespace std;

Redis::Redis()
    : _publishContext(nullptr)
    , _subcribeContext(nullptr)
{}

Redis::~Redis()
{
    if (_publishContext != nullptr)
        redisFree(_publishContext);

    if (_subcribeContext != nullptr)
        redisFree(_subcribeContext);
}

bool Redis::connect()
{
    // 负责 publish 发布消息的上下文连接
    _publishContext = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publishContext) {
        std::cerr << "Connect redis failed!" << endl;
        return false;
    }

    // 负责 subscribe 订阅消息的上下文连接
    _subcribeContext = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subcribeContext) {
        std::cerr << "Connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中, 监听通道上的事件, 有消息则上报给业务层
    thread t([&]() {
        observerChannelMessage();
    });
    t.detach();

    cout << "Connect redis-server success!" << endl;

    return true;
}

// 向 redis 指定的通道 channel 发布消息
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publishContext, 
        "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply) {
        std::cerr << "Publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向 redis 指定的通道 subscribe 订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE 命令本身会造成线程阻塞等待通道里面发生消息, 这里只订阅通道, 不接收通道消息
    // 通道消息的接收专门在 observerChannelMessage 函数的独立线程中进行
    // redisAppendCommand 会将 Redis 命令附加到上下文对象的输出缓冲区
    if (REDIS_ERR == redisAppendCommand(this->_subcribeContext, 
        "SUBSCRIBE %d", channel)) {
        std::cerr << "Subscribe command failed!" << endl;
        return false;
    }

    // redisBufferWrite 会将 Redis 上下文对象的输出缓冲区中的数据写入到 Redis 服务器
    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribeContext, &done)) {
            std::cerr << "Subscribe command failed!" << endl;
            return false;
        }
    }

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribeContext, 
        "UNSUBSCRIBE %d", channel)) {
        std::cerr << "Unsubscribe command failed!" << endl;
        return false;
    }

    int done = 0;
    while (!done) {
        if (REDIS_ERR == redisBufferWrite(this->_subcribeContext, &done)) {
            std::cerr << "Unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observerChannelMessage()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribeContext, (void **)&reply)) {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr 
            && reply->element[2]->str != nullptr)
            // 向业务层上报通道上发生的消息
            _notifyMessageHandler(atoi(reply->element[1]->str), reply->element[2]->str);

        freeReplyObject(reply);
    }

    std::cerr << ">>>>>>>>>>>>> observerChannelMessage quit <<<<<<<<<<<<<" << endl;
}

void Redis::initNotifyHandler(function<void(int,string)> fn)
{
    this->_notifyMessageHandler = fn;
}