#include"zookeeperutil.h"
#include"mprpcapplication.h"
#include<semaphore.h>
#include<iostream>

//全局的watcher观察器  zkserver给zkclient的通知
void global_watcher(zhandle_t *zh,int type,int state,const char *path,void *watcherCtx)
{
    if(type==ZOO_SESSION_EVENT)//回调的消息类型是和会话相关的消息类型
    {
        if(state==ZOO_CONNECTED_STATE)//zkclient和zkserver链接成功
        {
            sem_t *sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);//信号量资源加一
        }
    }
}

ZkClient::ZkClient():m_zhandle(nullptr)
{}

ZkClient::~ZkClient()
{
    if(m_zhandle!=nullptr)
    {
        zookeeper_close(m_zhandle);//关闭句柄释放资源
    }
}

//zkclinet启动链接zkserver
void ZkClient::Start()
{
    //加载zk的IP和端口号，默认为2181
    std::string host=MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port=MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr=host+":"+port;

    //调用原生API，端口与IP，回调函数，会话超时时间
    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程
    网络I/O线程：专门在一个线程里处理网络I/O
    watcher回调线程
    */
    m_zhandle=zookeeper_init(connstr.c_str(),global_watcher,30000,nullptr,nullptr,0);
    if(nullptr==m_zhandle)
    {
        std::cout<<"zookeeper_init error!"<<std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem,0,0);//初始化资源为0
    zoo_set_context(m_zhandle,&sem);//设置上下文，添加额外信息

    sem_wait(&sem);
    std::cout<<"zookeeper_init success!"<<std::endl;

}
//在zkserver上根据指定的path创建znode节点
void ZkClient::Create(const char *path,const char *data,int datalen,int state)
{
    char path_buffer[128];
    int bufferlen=sizeof(path_buffer);
    int flag;

    //检查该节点是否存在
    flag=zoo_exists(m_zhandle,path,0,nullptr);
    if(ZNONODE==flag)//该节点并不存在
    {
        //创建指定path的znode节点
        flag=zoo_create(m_zhandle,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buffer,bufferlen);
        if(flag==ZOK)
        {
            std::cout<<"znode create success... path:"<<path<<std::endl;
        }
        else
        {
            std::cout<<"flag:"<<flag<<std::endl;
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
//传入参数指定的znode节点路径，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen=sizeof(buffer);
    int flag=zoo_get(m_zhandle,path,0,buffer,&bufferlen,nullptr);//获取信息与返回值
    if(flag!=ZOK)//如果获取失败
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    }
    else
    {
        //获取成功
        return buffer;
    }
    
}
