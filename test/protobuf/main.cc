#include"test.pb.h"
#include<iostream>
#include<string>

using namespace rpcpr;

int main()
{
    // LoginResponse rsp;
    // ResultCode *rc=rsp.mutable_result();//使用指针指向rsp的结果
    // rc->set_errcode(1);//置为1
    // rc->set_errmsg("登陆失败");//输出错误信息

    GetFriendListsResponse rsp;
    ResultCode *rc =rsp.mutable_result();
    rc->set_errcode(0);

    User *user1=rsp.add_friend_list();//添加好友
    user1->set_name("zhangsan");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    std::cout<<rsp.friend_list_size()<<std::endl;
    return 0;
}

// int main1()
// {
//     LoginRequest req;//根据生成的cc文件定义类对象
//     req.set_name("zhangsan");//添加成员变量
//     req.set_pwd("123456");//添加成员变量

//     std::string send_str;//需要序列化的字符串
//     if(req.SerializeToString(&send_str))//如果序列化成功则执行以下代码
//     {
//         std::cout<<send_str<<std::endl;
//     }

//     //从send_str反序列化一个login请求对象
//     LoginRequest reqB;
//     if(reqB.ParseFromString(send_str))
//     {
//         std::cout<<reqB.name()<<std::endl;
//         std::cout<<reqB.pwd()<<std::endl;
//     }

//     return 0;
// }