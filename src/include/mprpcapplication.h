#pragma once

#include"mprpcconfig.h"
#include"mprpccontroller.h"
#include"mprpcchannel.h"
//mprpc框架的基础类，负责框架的一些初始化操作，使用单例模式设计
class MprpcApplication
{
public:
    static void Init(int argc,char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;
};