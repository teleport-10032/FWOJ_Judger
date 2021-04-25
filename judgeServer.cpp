#include"tcp_pthread_server.hpp"
#include <iostream>
#include <queue>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <fstream>
#include <stdio.h>

using namespace std;
string read(string file)
{
    FILE *fp;
    int c;
    string str;
    fp = fopen(file.c_str(),"r");
    while(1)
    {
        c = fgetc(fp);
        if( feof(fp) )
        {
            break ;
        }
        str += c;
    }
    fclose(fp);
    return str;
}
void HttpProcess(string& req, string* resp)
{
  int f1s = req.find("1teleports"),f1e = req.find("1teleporte");
  int f2s = req.find("2teleports"),f2e = req.find("2teleporte");
  int flagLen = 10;
  string code = req.substr(f1s+flagLen,f1e-f1s-flagLen);
  string problemId = req.substr(f2s+flagLen,f2e-f2s-flagLen);

  ofstream fout("judger/Main.cpp");
  fout << code;
  fout.close();

  ofstream fout2 ("judger/problemId.data");
  fout2 << problemId;
  fout2.close();

  pid_t pid = fork();
  if(0==pid){
      string source = "/home/teleport/Documents/FWOJ_BE/uploadFolder/test_case/" + problemId;
      string target = "/home/teleport/Documents/FWOJ_BE/FWOJ_Judger/judger";
      string shell = "cp -r " + source + " " + target;
      string cleanShell = "rm -rf " + target + "/" + problemId;
      char * shellStr = const_cast<char *>(shell.data());
      char * cleanShellStr = const_cast<char *>(cleanShell.data());
      system(shellStr);
      system("docker run --rm --cap-add=SYS_PTRACE  -v /home/teleport/Documents/FWOJ_BE/FWOJ_Judger/judger:/judger yuukiiiqwq/fwoj_judger:v2 bash -c 'cd /judger && g++ judger.cpp -o judger -std=c++11 && ./judger'");
      system(cleanShellStr);
      exit(0);
  }
  else if(-1!=pid){
      waitpid(pid,nullptr,0);
  }

  string ceInfo = read("judger/ce.log");

  string first_line = "HTTP/1.0 200 OK\n";
  string body = "";

  if(ceInfo.empty())
      body = read("judger/judger.log");
  else
      body = ceInfo;
  string header = "Content-Type: text/html\ncharset: gbk\nContent-Length:"
              +to_string(body.size())+"\n\n";


  // cout << "------------------------------------------------------" << endl;
  // cout << code << endl;
  // cout << "------------------------------------------------------" << endl;
  // cout << problemId << endl;
  // cout << "------------------------------------------------------" << endl;
  *resp = first_line + header + body;
}

int main()
{
  TcpPthreadServer server("0.0.0.0",8081);
  server.Start(HttpProcess);
  return 0;
}
