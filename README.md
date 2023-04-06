# MySkipList
## Description
Rewrite from Skiplist-CPP by Carl

What are differences between MySkiplist and Skiplist-CPP(author Carl): 
1. add span(SkipListNode::span_)
2. 添加 LRU 算法以淘汰过期数据
3. 使用 libhv 跨平台网络库作为 RPC 调用的基座（尚未完成）
4. 使用 epoll 网络通信模型实现了远端对键值对的增删改查操作

## Usage
### libhv
https://hewei.blog.csdn.net/article/details/119966701
1. export LD_LIBRARY_PATH=/home/gqy/codes/github/MySkiplist/lib:$LD_LIBRARY_PATH 将 libhv.so 链接进 LD_LIBRARY_PATH
2. 在项目根目录 make protorpc ，编译 protorpc
3. 启动服务端 bin/protorpc_server 1234 ，启动客户端 bin/protorpc_client 127.0.0.1 1234 add 1 2
4. 检查打印是否服务预期

### MySkipList
1. 在项目根目录，使用 make server 指令编译服务器端
2. 使用 make client 指令编译客户端
3. 使用 ./bin/server [PORT] 启动服务器端
4. 使用 ./bin/client [IP] [PORT] 启动客户端
5. 发送指令，当前支持以下指令类型：
```shell
# ZADD(zadd): 增加/更新有序集合中的成员
ZADD [key] [score] [member]
# ZREM(zrem): 删除有序集合中的成员
ZREM [key] [member]
# ZRCORE(zscore): 获取有序集合中的元素的成员的分数值
ZSCORE [key] [member]
# ZCARD(zcard): 获取有序集合中元素的数量
ZCARD [key]
# ZDIS(zdis): 打印有序集合
# 说明：目前该功能只支持服务器端查看
ZDIS [key]
# EXPIRE(expire): 设置键的过期时间
EXPIRE [key] [seconds]
```
