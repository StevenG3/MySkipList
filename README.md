# MySkipList
## Description
rewrite from Skiplist-CPP by Carl
What are differences between MySkiplist and Skiplist-CPP(author Carl)
1. add span(SkipListNode::span_)
2. 添加 LRU 算法以淘汰过期数据
3. 使用 libhv 跨平台网络库作为 RPC 调用的基座

## Usage
### libhv
https://hewei.blog.csdn.net/article/details/119966701
1. export LD_LIBRARY_PATH=/home/gqy/codes/github/MySkiplist/lib:$LD_LIBRARY_PATH 将 libhv.so 链接进 LD_LIBRARY_PATH
2. 在项目根目录 make protorpc ，编译 protorpc
3. 启动服务端 bin/protorpc_server 1234 ，启动客户端 bin/protorpc_client 127.0.0.1 1234 add 1 2
4. 检查打印是否服务预期

### MySkipList
