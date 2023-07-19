# TinyRpc

### 简介

- 基于重写的muduo库、protobuf、zookeeper实现的简易Rpc服务框架
- 以zookeeper作为RPC服务配置中心，RPC服务提供端开启服务时向zookeeper服务器注册/[ServiceName]/[MethodName]节点，节点保存的数据为[service_ip:service_port]
- 基于protobuf自定义Header解决粘包问题
- 手写muduo库实现了muduo网络IO的核心功能，并实现了日志系统。



### 安装

```shell
./autobuild
```