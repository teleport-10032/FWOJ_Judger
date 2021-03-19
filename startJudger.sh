#!/bin/bash

docker run --rm --cap-add=SYS_PTRACE  -v /home/teleport/CLionProjects/test2/judger:/judger fwoj_judger:v2 bash -c "cd judger && g++ judger.cpp -o judger -std=c++11 && ./judger"