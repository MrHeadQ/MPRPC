//本地服务发布成RPC服务的方法

#include<iostream>
#include<string>

#include"rpcprovider.h"
#include"mprpcapplication.h"
#include"../user.pb.h"//包含protobuf头文件

//using namespace RPC;
/*
UserService原来是一个本地服务，提供了两个进程内的本地方法
Login和GetFriendList
*/
//继承了RPC::UserServiceRpc,就封装成了一个RPC方法
class UserService:public RPC::UserServiceRpc
{
public:
    //登入系统的方法
    bool Login(std::string name,std::string pwd)
    {
        std::cout<<"doing local service:Login"<<std::endl;
        std::cout<<"name:"<<name<<" pwd"<<pwd<<std::endl;
        return true;
    }

    //新增的测试方法
    bool Register(uint32_t id,std::string name,std::string pwd)
    {
        std::cout<<"doing Register service:Login"<<std::endl;
        std::cout<<"id:"<<id<<" name:"<<name<<" pwd:"<<pwd<<std::endl;
        return true;
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::RPC::RegisterRequest* request,
                       ::RPC::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id=request->id();
        std::string name=request->name();
        std::string pwd=request->pwd();

        //开始做本地业务
        bool ret=Register(id,name,pwd);

        //填充回调结果
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
    /*
    重写基类UserServiceRpc的虚函数，以下方法由框架直接调用
    1.caller远程调用者发起远程调用请求Login(LoginRequest)=>muduo=>callee
    2.callee发现远程请求调用Login(LoginRequest)=>交付给下面这个重写的方法

    1.从LoginRequest获取参数的值
    2.执行本地服务Login，并获取返回值
    3.用上面的返回值填写LoginResponse
    4.一个回调，把LoginResonse发送给发起RPC服务的主机
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::RPC::LoginRequest* request,
                       ::RPC::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        //框架给业务上报了请求参数：LoginRequest，应用程序取出相应的已反序列化的数据来做本地业务
        std::string name=request->name();
        std::string pwd=request->pwd();

        //做本地业务
        bool loginresult=Login(name,pwd);

        //把响应写入,包括错误码，错误信息和运行结果
        RPC::ResultCode *Code=response->mutable_reslut();
        Code->set_errcode(1);
        Code->set_errmsg("Login do error!");
        response->set_success(loginresult);

        //执行回调操作  执行响应对象数据的序列化和网络发送
        done->Run();
    }

};

int main(int argc,char **argv)
{
    //先调用框架的初始化操作 provider -i config.conf，从init方法读取配置服务，比如IP地址和端口号
    MprpcApplication::Init(argc,argv);

    //项目提供者，让我们可以发布该服务
    RpcProvider provider;
    //把UserService对象发布到rpc节点上
    provider.NotifyService(new UserService());
    
    //启动一个rpc服务发布节点，run以后，进程进入阻塞状态，等待远程的rpc请求
    provider.Run();

    return 0;
}