#include<iostream>
#include"mprpcapplication.h"
#include"friend.pb.h"

int main(int argc,char **argv)
{
    //整个程序启动以后，想使用mprpc来获取rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);

    //演示调用远程发布的rpc方法Login
    RPC::FriendServiceRpc_Stub stub(new MprpcChannel);

    //rpc方法的请求参数
    RPC::GetFriendListRequest request;
    request.set_userid(1000);

    //rpc方法的响应
    RPC::GetFriendListResponse response;
    MprpcController controller;
    //发起rpc方法的调用，同步的rpc调用过程，MprpcChannel::callmethod
    stub.GetFriendList(&controller,&request,&response,nullptr);
    //stub.Login();//RpcChannel->RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送

    //一次rpc调用完成，读取调用的结果
    if(controller.Failed())
    {
        std::cout<<controller.ErrorText()<<std::endl;
    }
    else
    {
        if(0==response.result().errcode())//由于调用不一定成功，所以不要直接访问他的reslut
        {
            std::cout<<"rpc GetFriendList response success!"<<std::endl;
            int size=response.friends_size();
            for(int i=0;i<size;++i)
            {
                std::cout<<"index: "<<(i+1)<<" name:"<<response.friends(i)<<std::endl;
            }
        }
        else
        {
            std::cout<<"rpc GetFriendList response error:"<<response.result().errmsg()<<std::endl;
        }
    }

    return 0;
}