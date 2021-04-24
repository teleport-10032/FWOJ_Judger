judgeServer:judgeServer.cpp
	g++ $^ -o $@ -std=c++11 -lpthread
.PHONY:clean
	rm -rf judgerServer
