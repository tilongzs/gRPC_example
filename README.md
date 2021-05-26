# gRPC_example

#### 介绍
gRPC的基本使用演示工程。包含同步模式和异步模式的基本使用示例。演示了全部的一对一、一对多、多对一、多对多模式，以及多线程。


#### 安装教程
通过vcpkg安装，并引用了cpprest库的task并行类。

- .\vcpkg install grpc
- .\vcpkg install cpprest

默认使用Debug、X64配置，以及C++17和C17标准编译。

#### 使用说明

gRPC_AsyncClient与gRPC_AsyncServer是两个控制台工程，异步gRPC框架的基本使用。来源是https://github.com/Mityuha/grpc_async ，对应着一位俄罗斯软件工程师的一篇博客https://habr.com/en/post/340758 ，对我的帮助非常大。为减少歧义，代码中的英文注释我就保持原样了。

gRPC_MFCClient与gRPC_MFCServer是两个MFC工程，包含同步模式和异步模式的基本使用示例。异步服务端默认是1个线程，可以根据需要开启多个线程（与CPU同等数量）。客户端则使用单线程，其实我觉得客户端使用同步模式更方便些。

#### 其他
博客原文：https://www.mengmei.moe/note/1015.html
编译问题请参考：《[gRPC-开发常用笔记](https://www.mengmei.moe/note/958.html)》
