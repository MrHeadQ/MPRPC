#include<iostream>
#include"mprpcapplication.h"
#include"user.pb.h"
#include"mprpcchannel.h"

int main(int argc,char **argv)
{
    //整个程序启动以后，想使用mprpc来获取rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    RPC::UserServiceRpc_Stub stub(new MprpcChannel);

    //rpc方法的请求参数
    RPC::LoginRequest request;
    request.set_name("zhangsan");
    request.set_pwd("123");

    //rpc方法的响应
    RPC::LoginResponse response;
    //发起rpc方法的调用，同步的rpc调用过程，MprpcChannel::callmethod
    stub.Login(nullptr,&request,&response,nullptr);
    //stub.Login();//RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    //一次rpc调用完成，读取调用的结果
    if(0==response.reslut().errcode())
    {
        std::cout<<"rpc login response:"<<response.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<response.reslut().errmsg()<<std::endl;
    }

    //演示RPC的register方法
    RPC::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    RPC::RegisterResponse rsp;
    
    //以同步的方式发起RPC请求，等待返回结果
    stub.Register(nullptr,&req,&rsp,nullptr);

    //一次rpc调用完成，读取调用的结果
    if(0==rsp.result().errcode())
    {
        std::cout<<"rpc login response:"<<rsp.success()<<std::endl;
    }
    else
    {
        std::cout<<"rpc login response error:"<<rsp.result().errmsg()<<std::endl;
    }

    return 0;
}