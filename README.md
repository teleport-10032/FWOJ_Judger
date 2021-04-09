一个简易OnlineJudge判题机demo，在docker容器中跑用户程序，只支持C++。

准备工作：

1. 由于要用到linux系统调用，请准备linux系统；
2. 安装docker(`sudo curl -sSL https://get.daocloud.io/docker | sh`)和docker-compose(`sudo apt-get install docker-compose`);
3. 使用`sudo docker pull yuukiiiqwq/fwoj_judger:v2`拉取镜像，更改startJudger.sh中的目录映射为自己本地的情况；
4. 使用`sudo ./startup.sh`进行判题。

目前没做接口，需要请自便。

1. 用户源代码:  main.cpp line 13
2. 测试数据: judger/judger.cpp  212  (inQueue和outQueue分别存放输入输出测试数据)

程序执行后将结果存储在judger/judger.log文件中。
若编译成功则返回各组样例的结果，时间和内存；若失败则返回"CE",错误详情在judger/ce.log。
