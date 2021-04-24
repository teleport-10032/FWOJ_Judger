#include<iostream>
#include<pthread.h>
#include<functional>
#include"tcp_socket.hpp"

using namespace std;
typedef function<void (string& req,string* resp)> Handler;
struct Arg{
  TcpSocket new_sock;
  string ip;
  uint16_t port;
  Handler handler;
};
class TcpPthreadServer
{
public:
  TcpPthreadServer(const string& ip, uint16_t port):_ip(ip),_port(port){}

  bool Start(Handler handler)
  {
    listen_sock.Socket();
    listen_sock.Bind(_ip,_port);
    listen_sock.Listen(5);
    while(1)
    {
      Arg* arg = new Arg();
      arg->handler = handler;
      if(!listen_sock.Accept(&arg->new_sock,&arg->ip,&arg->port)){
        continue;
      }
      pthread_t tid;
      pthread_create(&tid,NULL,ThreadEntry,arg);
      pthread_detach(tid);
    }
    return true;

  }
  static void* ThreadEntry(void* arg)
  {
    Arg* Targ = (Arg*)arg;
    while(1)
    {
      string req;
      bool ret = Targ->new_sock.Recv(&req);
      if(!ret){ 
        break;
      }  
      string resp;
      Targ->handler(req,&resp);
      Targ->new_sock.Send(resp);
    }
    Targ->new_sock.Close();
    Targ = nullptr;
    return nullptr;
  }
private:
  TcpSocket listen_sock;
  string _ip;
  uint16_t _port;
};
