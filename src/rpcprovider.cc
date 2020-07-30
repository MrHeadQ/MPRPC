#include"rpcprovider.h"
#include"mprpcapplication.h"
#include"rpcheader.pb.h"
#include"zookeeperutil.h"

/*
service_name=>对service进行描述
            1.service* 记录服务对象
            2.method_name 记录服务方法对象
*/
//这里是框架提供给外部使用的，可以发布rpc方法的函数接口
//此处应该使用Service类，而不是指定某个方法
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;//服务表
    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc=service->GetDescriptor();
    //获取服务的名字
    std::string service_name=pserviceDesc->name();
    //获取服务对象service的方法数量
    int methodCnt=pserviceDesc->method_count();

    //std::cout<<"service name:"<<service_name<<std::endl;添加日志信息后更改
    LOG_INFO("service_name:%s",service_name.c_str());

    for(int i=0;i<methodCnt;++i)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc=pserviceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        //std::cout<<"method name:"<<method_name<<std::endl;
        //插入服务
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        LOG_INFO("method_name:%s",method_name.c_str());
    }
    //可以使用该表来调用方法
    service_info.m_service=service;
    m_serviceMap.insert({service_name,service_info});
}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run()
{
    std::string ip=MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port=atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip,port);

    //创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    //绑定链接回调和消息读写回调方法,很好的分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessge,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    //设置muduo库的线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册在zk上，让rpc client可以从zk上发现服务
    //session的timeout默认为30s，zkclient的网络I/O线程1/3的timeout内不发送心跳则丢弃此节点
    ZkClient zkCli;
    zkCli.Start();//链接zkserver
    for(auto &sp:m_serviceMap)
    {
        //service_name
        std::string service_path ="/"+sp.first;//拼接路径
        zkCli.Create(service_path.c_str(),nullptr,0);//创建临时性节点
        for(auto &mp:sp.second.m_methodMap)
        {
            //service_name/method_name
            std::string method_path=service_path+"/"+mp.first;//拼接服务器路径和方法路径
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);//向data中写入路径

            //创建节点,ZOO_EPHEMERAL表示临时节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    std::cout<<"RpcProvider start service at ip:"<<ip<<" port:"<<port<<std::endl;
    //启动网络服务
    server.start();
    m_eventLoop.loop();//启动epollwait
}

//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        //和rpcclient的链接断开了
        conn->shutdown();
    }
}

/*
在框架内部需要提前协商好通信使用的protobuf数据类型：比如发送过来的数据类型为：service_name,method_name,args
需要定义proto的message类型，进行数据头的序列化和反序列化，为防止TCP的粘包，需要对各个参数进行参数的长度明确

定义header_size（4字节） + header_str + args_str

已建立连接的用户的读写事件回调，网络上如果有一个远程的rpc服务请求，则onmessge方法就会响应
*/
void RpcProvider::OnMessge(const muduo::net::TcpConnectionPtr &conn,
                                    muduo::net::Buffer *buffer,
                                    muduo::Timestamp)
{
    //获取到数据，即网络上接受的远程rpc调用请求的字符流， Login和args
    std::string recv_buf=buffer->retrieveAllAsString();

    //std::cout<<"已获取数据"<<std::endl;
    //读取header_size，此时的整数若按照字符串格式发送，读取时会出现问题，所以需要直接按二进制发送
    //从字符流中读取前四个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);
    //std::cout<<"已读取头信息"<<std::endl;
    //根据header_size读取数据头的原始字符流，反序列化数据，得到rpc的详细请求数据
    std::string rpc_header_str=recv_buf.substr(4,header_size);//substr从4开始读读取header_size个字节的数据
    mprpc::RpcHeader rpcHeader;
    std::string service_name;//用于存储反序列化成功的服务名字
    std::string method_name;//用于存储反序列化成功的服务方法
    uint32_t args_size;//用于存储反序列化成功的参数个数
    //std::cout<<"已读取头信息"<<std::endl;
    std::cout<<"======================================"<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str"<<rpc_header_str<<std::endl;
    std::cout<<"======================================"<<std::endl;
    if(rpcHeader.ParseFromString(rpc_header_str))//开始反序列化，参数接受类型为引用,返回值为bool型
    {
        //数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout<<"rpc_header_str: "<<rpc_header_str<<"parse error!"<<std::endl;
        return;
    }

    //获取rpc参数方法的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    //打印调试信息
    std::cout<<"======================================"<<std::endl;
    std::cout<<"header_size: "<<header_size<<std::endl;
    std::cout<<"rpc_header_str"<<rpc_header_str<<std::endl;
    std::cout<<"service_name: "<<service_name<<std::endl;
    std::cout<<"method_name: "<<method_name<<std::endl;
    std::cout<<"args_str: "<<args_str<<std::endl;
    std::cout<<"======================================"<<std::endl;

    //获取service对象和method对象
    auto it =m_serviceMap.find(service_name);
    if(it==m_serviceMap.end())
    {
        //如果方法不存在
        std::cout<<service_name<<"is not exist!"<<std::endl;
        return;
    }

    auto mit=it->second.m_methodMap.find(method_name);
    if(mit==it->second.m_methodMap.end())
    {
        //如果服务提供的方法不存在
        std::cout<<service_name<<":"<<method_name<<"is not exists!"<<std::endl;
    }
    google::protobuf::Service *service=it->second.m_service;//获取service对象，对应Userservice
    const google::protobuf::MethodDescriptor *method=mit->second;//获取method对象，对应Login方法

    //生成rpc方法调用的请求request和相应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();//生成一个新对象
    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error,content:"<<args_str<<std::endl;
        return;
    }

    google::protobuf::Message *response = service->GetResponsePrototype(method).New();//生成一个新对象

    //给下面的method方法的调用，绑定一个Closure的回调函数，因为模板的实参推演失败，所以需要指定类型
    google::protobuf::Closure *done=google::protobuf::NewCallback
    <RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>
    (this,&RpcProvider::SendRpcResponse,conn,response);
    //在框架上根据远端rpc请求，调用当前rpc节点上发布的方法

    //相当于UserService调用了Login方法
    service->CallMethod(method,nullptr,request,response,done);
}

//Closure的回调操作，用于序列化rpc的相应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,google::protobuf::Message* response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))//response进行序列化
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送回rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout<<"Serialize response error!"<<std::endl;
    }
    //模拟http的短链接服务，由rpcprovider主动断开连接
    conn->shutdown();
}