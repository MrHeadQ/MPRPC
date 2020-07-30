#pragma once
#include"google/protobuf/service.h"
//muduo库的头文件
#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include<google/protobuf/descriptor.h>
#include<functional>
#include<unordered_map>
#include<string>
#include"logger.h"

//框架提供的专门负责发布rpc服务的网络对象类
class RpcProvider
{
public:
    //这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    //此处应该使用Service类，而不是指定某个方法
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
private:
    
    muduo::net::EventLoop m_eventLoop;

    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;//保存服务对象
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*> m_methodMap;//保存服务方法

    };

    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string,ServiceInfo> m_serviceMap;
    //新的socket链接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //已建立连接用户的读写事件回调
    void OnMessge(const muduo::net::TcpConnectionPtr&,muduo::net::Buffer*,muduo::Timestamp);
    //Closure的回调操作，用于序列化RPC的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message* );
};