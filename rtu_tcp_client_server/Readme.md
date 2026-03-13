# 项目名称
rtu_tcp_client_server

## 简介
**项目背景** 在Windows11操作系统上安装了MSYS2，在其上成功编译了libmodbus(版本3.1.12没有编译成功，版本3.1.6编译成功)。基于MSYS2+MinGW环境，开发了测试libmodbus的程序。用于验证最基础的libmodbus使用方法。借助 Copilot 生成了Windows + Linux的跨平台工程。在MSYS2系统下对程序进行了验证。

**目标** 验证MSYS2系统下，libmodbus库的使用方法。原来在MSYS2系统下，可以直接使用Windows系统下的串口资源，包括用“虚拟串口软件”虚拟出来的串口。本项目包括4个程序，分别是：rtu_client, rtu_server, tcp_client, tcp_server

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
在Windows系统中打开串口软件，虚拟一对串口COM5<->COM6。

启动Modbus Slave软件，在COM5上建立Modbus通信，参数为：com5,N,8,1,RTU

进入build目录，执行命令
```bash
$ cd build
$ ./rtu_server.exe
```

将得到如下内容的回显
```bash
RTU server started, writing holding registers...
Server HR[0..9]: 741 2197 9802 1674 7629 7309 5256 6469 4309 3888
Server HR[0..9]: 4540 2592 5353 4811 2416 896 2403 6020 2534 4096
Server HR[0..9]: 9484 6986 3738 2396 7701 737 1112 9536 4789 5885
Server HR[0..9]: 1421 1007 5875 3997 2578 2712 3686 1530 3382 8406
Server HR[0..9]: 6074 1184 2731 3592 2055 9132 9460 8208 8045 8070
Server HR[0..9]: 926 5791 7666 5313 1501 1889 2496 3553 6775 3877
Server HR[0..9]: 2488 2444 4359 7280 3643 2644 2251 1810 6539 292
Server HR[0..9]: 335 2681 3309 2489 1037 581 1154 4083 69 7388
Server HR[0..9]: 9958 5692 6682 7166 1032 4344 2845 1851 8843 7294
```

再开启一个终端，执行如下命令
```bash
$ cd build
$ ./rtu_server.exe
```

将得到如下内容的回显
```bash
RTU client started, reading holding registers...
Client HR[0..9]: 4540 2592 5353 4811 2416 896 2403 6020 2534 4096
Client HR[0..9]: 9484 6986 3738 2396 7701 737 1112 9536 4789 5885
Client HR[0..9]: 1421 1007 5875 3997 2578 2712 3686 1530 3382 8406
Client HR[0..9]: 6074 1184 2731 3592 2055 9132 9460 8208 8045 8070
Client HR[0..9]: 926 5791 7666 5313 1501 1889 2496 3553 6775 3877
Client HR[0..9]: 2488 2444 4359 7280 3643 2644 2251 1810 6539 292
```

对比两个终端的输出，可以看到rtu_server生成的数据，被rtu_client正确收到。


## 注意事项
程序运行时需要根据计算机上具体的COM口编号进行修改