#include <iostream>
#include <queue>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <fstream>

int main(){

    freopen("main.log","a",stdout);
    std::queue<std::string> codeQueue;
    codeQueue.push("#include <iostream>\n"
                   "\n"
                   "using namespace std;\n"
                   "\n"
                   "int main()\n"
                   "{\n"
                   "    int a , b; cin >> a >> b;\n"
                   "    int c = 222;\n"
                   "    for(int i =0 ; i < 105; i ++)\n"
                   "        if(i == 1002)\n"
                   "            cout << c << endl;\n"
                   "    cout << a + b << endl;\n"
                   "    return 0;\n"
                   "}");
    while(!codeQueue.empty()) {
        std::string code = codeQueue.front();
        codeQueue.pop();

        std::ofstream fout("judger/Main.cpp");
        fout << code;
        fout.close();

        pid_t pid = fork();
        if(0==pid){
            execl("./startJudger.sh","");
            exit(0);
        }
        else if(-1!=pid){
            waitpid(pid,nullptr,0);
        }
        else{
            std::cout << "error!\n";
            return -1;
        }
    }
    return 0;
}
