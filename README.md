# 基于UDP的TFTP文件传输项目
## 一、TFTP传输模型的简介
基于UDP通信的TFTP（Trivial File Transfer Protocol，简单文件传输协议）模型是一种用于在网络上进
行文件传输的简单协议。TFTP设计简单，易于实现，通常用于传输小文件或在不支持复杂协议的环境中
使用
### 1.1 协议概述
TFTP使用UDP（User Datagram Protocol，用户数据报协议）作为传输层协议，默认端口号为69。
TFTP协议设计简单，仅支持五种类型的报文，分别是：
- RRQ（Read Request）：客户端请求读取文件。
- WRQ（Write Request）：客户端请求写入文件。
- DATA：数据传输报文。
- ACK：确认报文。
- ERROR：错误报文。
### 1.2 通信流程
通信流程一般分为请求阶段和传输阶段
#### 1.2.1 请求阶段
- 客户端发送读写请求数据包：客户端向服务器发起RRQ或WRQ报文，其中包含文件名和传输模式（通常是“netascii”或“octet”）
- 服务器响应：服务器收到RRQ或WRQ报文后，会分配一个UDP端口用于数据传输并发送对应的数据包或ACK包
#### 1.2.2 传输阶段
- 数据传输：在RRQ请求中，服务器开始发送DATA报文，每个DATA报文包含一个块编号和数据块。
客户端接收到DATA报文后，发送ACK报文确认接收到的块编号。在WRQ请求中，客户端发送DATA
报文，服务器发送ACK报文确认。
- 块编号：每个DATA报文和ACK报文都包含一个16位的块编号，从1开始递增。块编号用于确保数据
的有序传输和确认。
- 结束传输：当传输的文件大小小于512字节时，DATA报文的数据部分小于512字节，表示传输结
束。对于WRQ请求，客户端发送一个小于512字节的DATA报文表示传输结束。
### 1.3 报文格式
| 读写请求 | 操作码1/2 (R/W) | 文件名    | 通信模式“netascii”或“octet” | 0 |
|---|---|---|---|---|
|         | 2 Byte         | nB string | 1B         | nB string | 1B |

| 数据包   | 操作码3        | 块编号    | 数据                      |
|---|---|---|---|
|         | 2 Byte         | 2 Byte    | 0到512B，实际传输的数据。    |

| ACK     | 操作码4 (ACK) | 块编号    |
|---|---|---|
|         | 2 Byte         | 2 Byte    |

| ERROR   | 操作码5        | 错误码    | 错误信息      | 0 |
|---|---|---|---|---|
|         | 2Byte          | 2Byte     | nB string    | 1B |
## 二、功能特性
- 支持 TFTP RRQ（读请求）操作
- 支持 TFTP WRQ（写请求）操作
- 支持 DATA、ACK、ERROR 报文处理
- 使用 UDP 进行无连接数据传输
- 实现标准 512 字节数据分块
- 支持文件二进制模式（octet）
- 支持基本错误状态返回（文件不存在、无权限、非法操作等）
- 提供命令行交互菜单
## 三、项目结构
├── LICENSE
├── README.md
├── tftp_client
│   ├── CMakeLists.txt
│   ├── header
│   │   └── tftp_client.h
│   └── src
│       ├── main.cpp
│       └── tftp_client.cpp
└── tftp_server
    |
    ├── CMakeLists.txt
    ├── header
    │   └── tftp_server.h
    └── src
        ├── main.cpp
        └── tftp_server.cpp

