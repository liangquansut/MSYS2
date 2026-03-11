# 项目名称
在rtu_tcp_client_server的基础上，修改工程为modbusPolling

## 简介
**更新日志** 在实现轮巡的基础上，添加数据处理线程，实现队列缓冲区数据通信

**项目背景** 在Windows11操作系统上安装了MSYS2，在其上成功编译了libmodbus(版本3.1.12没有编译成功，版本3.1.6编译成功)。基于MSYS2+MinGW环境，开发了测试libmodbus的程序。用于验证最基础的libmodbus使用方法。借助 Copilot 生成了Windows + Linux的跨平台工程。在MSYS2系统下对程序进行了验证。

**目标** 验证MSYS2系统下，libmodbus库的使用方法。原来在MSYS2系统下，可以直接使用Windows系统下的串口资源，包括用“虚拟串口软件”虚拟出来的串口。本项目包括4个程序，分别是：rtu_poll, rtu_server, tcp_poll, tcp_server。设计本程序的目的，是实现rtu_poll和rtu_server通信；tcp_poll和tcp_server通信

## 目录结构
- .vscode/：vscode配置文件目录
- include/：头文件目录
    - common.h：通用头文件
- src/：源文件目录

## 环境配置
- Windows操作系统
- Windows下的虚拟串口软件
- MSYS2
- 在MSYS2中安装：autoconf automake libtool
- 编译好的libmodbus库文件

## 运行方式
在Windows系统中打开串口软件，虚拟一对串口COM5<->COM6

启动Modbus Slave软件，在COM5上建立Modbus通信，参数为：com5,N,8,1,RTU

进入build目录，执行程序
```bash
$ cd build
$ ./app.exe
```

将得到如下内容的回显
```bash
[01][03][00][00][00][01][84][0A]
Waiting for a confirmation...
<01><03><02><00><0F><F8><40>
Register[0] = 15
```

## 注意事项
程序运行时需要根据计算机上具体的COM口编号进行修改