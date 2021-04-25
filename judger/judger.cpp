#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/reg.h>
#include <unistd.h>
#include <queue>
using namespace std;

const int ALLOW_SYS_CALL_C[] = {0,1,2,3,4,5,8,9,11,12,20,21,59,63,89,158,231,240, SYS_time, SYS_read, SYS_uname, SYS_write
        , SYS_open, SYS_close, SYS_execve, SYS_access, SYS_brk, SYS_munmap, SYS_mprotect, SYS_mmap, SYS_fstat
        , SYS_set_thread_area, 252, SYS_arch_prctl, 0 };

bool allowSysCall[1024];
string result = "AC";


int compile(const char* ceInfoFile){
    int time_limit = 10;
    int memory_limit = 128 * 1024 * 1024;
    const char *COMP_CPP[] = {"g++","-Wall","-fno-asm","-lm", "--static", "-std=c++11"
            ,"Main.cpp","-o","Main",nullptr};
    pid_t pid = fork();
    if (pid == -1){
        cout << "SE\n" ;
        return -1;
    } else if (pid != 0) {
        int status;
        waitpid(pid, &status, 0);
        return status;
    } else {
        struct rlimit lim;
        lim.rlim_cur = lim.rlim_max = time_limit;
        setrlimit(RLIMIT_CPU, &lim);
        alarm(0);
        alarm(time_limit);
        lim.rlim_cur = memory_limit;
        setrlimit(RLIMIT_AS, &lim);
        lim.rlim_cur = lim.rlim_max = 10 * 1024 * 1024;
        setrlimit(RLIMIT_FSIZE, &lim);
        freopen(ceInfoFile,"w",stderr);
        execvp(COMP_CPP[0],(char* const*)COMP_CPP);
        exit(0);
    }
}

int getMem(int pid, const char* field){
    char file[105];
    sprintf(file, "/proc/%d/status", pid);
    ifstream fin(file);
    string line;
    int fieldLen = 0,lineLen;
    fieldLen = strlen(field);
    int num = 0;
    while (getline(fin, line)) {
        lineLen = line.length();
        if (lineLen <= fieldLen) continue;
        bool flag = true;
        for (int i = 0; i < fieldLen; ++i) {
            if (line[i] != field[i]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            for (int i = fieldLen; i < lineLen; ++i) {
                if (line[i] >= '0' && line[i] <= '9') num = num * 10 + line[i] - '0';
                else if (num) break;
            }
            break;
        }
    }
    return num;
}


void run(int timeLimit, int memLimit, int& usedTime, const char* inDataFile
        , const char* userOutFile) {
    freopen(inDataFile, "r", stdin);
    freopen(userOutFile, "w", stdout);
    ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
    struct rlimit lim;

    lim.rlim_cur = lim.rlim_max = timeLimit/1000.0+1;
    setrlimit(RLIMIT_CPU, &lim);
    alarm(0);
    alarm(timeLimit/1000.0+1);

    lim.rlim_max = (10 * 1024 * 1024) + 1024 * 1024;
    lim.rlim_cur = 10 * 1024 * 1024;
    setrlimit(RLIMIT_FSIZE, &lim);

    lim.rlim_cur = lim.rlim_max = 1;
    setrlimit(RLIMIT_NPROC, &lim);

    lim.rlim_cur = lim.rlim_max = (1024 * 1024) << 6;
    setrlimit(RLIMIT_STACK, &lim);
    lim.rlim_cur = memLimit * (1024 * 1024) / 2 * 3;
    lim.rlim_max = memLimit * (1024 * 1024);
    setrlimit(RLIMIT_AS, &lim);
    execl("./Main", "./Main", nullptr);
}

void updateStatus(pid_t pid, int& topMem
        , int& usedTime, int memLimit, int timeLimit) {
    int maxMem = 0;
    int status, sig, exitCode;
    struct rusage usage;

    if (topMem == 0)
        topMem = getMem(pid, "VmRSS:")<<10;

    wait(&status);
    if (WIFEXITED(status)) return;

    ptrace(PTRACE_SETOPTIONS, pid, nullptr
            , PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXIT | PTRACE_O_EXITKILL);
    ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);

    while (true) {
        wait4(pid, &status, __WALL, &usage);

        maxMem = getMem(pid, "VmPeak:") << 10;

        if (maxMem > topMem) topMem = maxMem;
        if (topMem > (memLimit<<20)) {
            if (result.compare("AC")) result = "MLE";
            ptrace(PTRACE_KILL, pid, nullptr, nullptr);
            break;
        }
        if (WIFEXITED(status)) break;
        exitCode = WEXITSTATUS(status);
        if ( !( exitCode==0 || exitCode==133 || exitCode==5) ) {
            if (result == "AC") {
                switch (exitCode) {
                    case SIGCHLD :
                    case SIGALRM :
                        alarm(0);
                    case SIGKILL :
                    case SIGXCPU :
                        result = "TLE";
                        break;
                    case SIGXFSZ :
                        result = "OLE";
                        break;
                    default :
                        result = "RE";
                }
            }
            ptrace(PTRACE_KILL, pid, nullptr, nullptr);
            break;
        }
        if (WIFSIGNALED(status)) {
            sig = WTERMSIG(status);
            if (result == "AC") {
                switch (sig) {
                    case SIGCHLD :
                    case SIGALRM :
                        alarm(0);
                    case SIGKILL :
                    case SIGXCPU :
                        result = "TLE";
                        break;
                    case SIGXFSZ :
                        result = "OLE";
                        break;
                    default :
                        result = "RE";
                }
            }
            ptrace(PTRACE_KILL, pid, nullptr, nullptr);
            break;
        }
        int sysCall = ptrace(PTRACE_PEEKUSER, pid, ORIG_RAX<<3, nullptr);
        if (!allowSysCall[sysCall]) {
            result = "RE";
            ptrace(PTRACE_KILL, pid, nullptr, nullptr);
            break;
        }
        ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
    }
    if (result == "TLE") usedTime = timeLimit;
    else {
        usedTime += (usage.ru_utime.tv_sec*1000 + usage.ru_utime.tv_usec/1000);
        usedTime += (usage.ru_stime.tv_sec*1000 + usage.ru_stime.tv_usec/1000);
    }
}

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

int main() {

    freopen("judger.log","w",stdout);
    int status = compile("ce.log");
//    cout << "start!\n" ;
    if (status == -1) {
//        cout << "SE!\n" ;
    } else if (status != 0) {
//        cout << "CE!\n" ;
    }
    else {
//        cout << "compileSuccess!\n" ;
        string inFile = "in.data";
        string userOutFile = "out.data";
        int timeLimit = 1000, memLimit = 128;

        string problemId =  read("problemId.data");
        string info = "test_case/"+problemId+"/info.data";
        string testcaseNum = read(info);

        queue<string> inQueue;
        queue<string> outQueue;

        int n = stoi(testcaseNum);

        for(int i = 1 ; i <= n ; i++)
        {
            string inStr = "test_case/"+problemId+"/"+to_string(i)+".in";
            string outStr = "test_case/"+problemId+"/"+to_string(i)+".out";
            inQueue.push(read(inStr));
            outQueue.push(read(outStr));
        }

        int maxTime = 0;
        int topMem = 0;

        result = "AC";

        memset(allowSysCall, false ,sizeof(allowSysCall));
        for (int i=0; !i || ALLOW_SYS_CALL_C[i]; ++i) {
            allowSysCall[ALLOW_SYS_CALL_C[i]] = true;
        }
        while (!inQueue.empty()) {
            string inputData = inQueue.front();
            inQueue.pop();
            string outputData = outQueue.front();
            outQueue.pop();
            result = "AC";
//        cout << "|" << inputData << "|" << outputData << "|" << endl;
            ofstream fout(inFile);
            fout << inputData;
            fout.close();
            int usedTime = 0;
            pid_t pidRun = fork();
            if (pidRun == 0) {
                run(timeLimit, memLimit, usedTime, inFile.c_str(), userOutFile.c_str());
                exit(0);
            } else if (pidRun != -1) {
                updateStatus(pidRun ,  topMem, usedTime
                        , memLimit, timeLimit);
                if (maxTime < usedTime)
                    maxTime = usedTime;
            }
            else
                result = "SE";
            ifstream fin(userOutFile);
            string line;
            string userOut = "";
            while (getline(fin,line)) {
                userOut += line+"\n";
            }
            fin.close();
            if(outputData.compare(userOut)!=0)
                result = "WA";
//        cout << "userOut:" << userOutStr << "|standardOut:" << outputData << "\n";
            cout << "result:" << result.c_str() <<",time:" << maxTime << ",mem:" << topMem << endl;
        }
    }
//    cout << "end!\n";
    return 0;
}
