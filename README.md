一个通过暴露http接口提供判题服务的判题机，以docker作为沙箱，传入源代码和题号，judger会在`judger/test_case`目录下查找测试样例，并返回判题结果或CE信息。

接口调用示例:

`http://10.211.55.6:8001/`

body内容

```
1teleports
#include "stdio.h"
int main()
{int a;int b;scanf("%d %d",&a,&b);printf("%d",1a);}1teleporte2teleports10002teleporte
```

>在该请求中传入了源程序和题号1000，奇怪的字符用来方便定位。

准备工作：

1. 由于要用到linux系统调用，请准备linux系统，并使用chmod设定好权限；
2. 安装docker(`sudo curl -sSL https://get.daocloud.io/docker | sh`)和docker-compose(`sudo apt-get install docker-compose`);
3. 使用`sudo docker pull yuukiiiqwq/fwoj_judger:v2`拉取镜像，更改judgerServer.cpp中的目录映射为自己本地的情况；
4. 使用`make && sudo ./judgerServer`启动判题服务。

