一个简易OnlineJudge判题机demo，在docker容器中跑用户程序，只支持C++。

准备工作：

1. 由于要用到linux系统调用，请准备linux系统；
2. 安装docker；
3. 拉取yuukiiiqwq/fwoj_judger:v2镜像，更改startJudger.sh中的目录映射为自己本地的情况；

目前没做接口，请手动改写参数。

1. 用户源代码:  main.cpp line 13
2. 测试数据: judger/judger.cpp  212  (inQueue和outQueue分别存放输入输出测试数据)

程序执行结果在judger/judger.log文件中，如果编译成功则返回各组样例的结果，时间和内存。

失败则返回文本"CE",编译详情在judger/ce.log