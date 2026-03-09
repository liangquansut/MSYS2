# 项目名称
vscode_libmodbus_test

## 简介
**项目背景** 在Windows11操作系统上安装了MSYS2，在其上成功编译了libmodbus(版本3.1.12没有编译成功，版本3.1.6编译成功)。基于MSYS2+MinGW环境，开发了测试libmodbus的程序。用于验证最基础的libmodbus使用方法。

**目标** 验证MSYS2系统下，libmodbus库的使用方法。原来在MSYS2系统下，可以直接使用Windows系统下的串口资源，包括用“虚拟串口软件”虚拟出来的串口。

## 目录结构
- .vscode/：vscode配置文件
- include/：头文件（本例为空）
- src/：源文件

## 环境配置
- Windows操作系统
- Windows下的虚拟串口软件
- MSYS2
- 在MSYS2中安装：autoconf automake libtool
- 编译好的libmodbus库文件

## 运行方式
在Windows系统中打开串口软件，虚拟一对串口COM5<->COM6。

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