#pragma once
#include"lockqueue.h"
#include<string>

enum LogLevel
{
    INFO,//普通日志信息
    ERROR,//错误信息
};

//mprpc框架提供的日志系统
class Logger
{
public:
    //获取日志的单例
    static Logger& GetInstance();
    void StLogLevel(LogLevel level);//设置日志级别
    void Log(std::string msg);//写日志
private:
    int m_loglevel;//记录日志级别
    LockQueue<std::string> m_lckQueue;//日志缓冲队列

    //设置成单例模式，删除掉其拷贝构造，构造函数设置成私有
    Logger();
    Logger(const Logger&)=delete;
    Logger(Logger&&)=delete;
};

//定义宏对用户实现自己写日志的效果
//
#define LOG_INFO(logmsgformat,...)\
    do\
    { \
        Logger &logger=Logger::GetInstance();\
        logger.StLogLevel(INFO);\
        char c[1024]={0};\
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c);\
    }while(0)

#define LOG_ERR(logmsgformat,...)\
    do\
    { \
        Logger &logger=Logger::GetInstance();\
        logger.StLogLevel(ERROR);\
        char c[1024]={0};\
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c);\
    }while(0)
    