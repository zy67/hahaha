#include<iostream>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<vector>
#include<string>
using std::cout;
using std::endl;
using std::cerr;


int StartTcp(int _port)
{
  int sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock<0)
  {
    cerr<<"sock error!"<<endl;
    return 1;
  }
  int opt=1;
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(int));
  sockaddr_in addr;
  addr.sin_port=htons(_port);
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(sock,(sockaddr*)&addr,sizeof(addr)))
  {
    cerr<<"bind error!"<<endl;
    return 2;
  }
  if(listen(sock,10))
  {
    cerr<<"listen error"<<endl;
    return 3;
  }
  return sock;
}


void EpollServer()
{
  int ser_sock=StartTcp(6060);
  int epoll_fd_=epoll_create(10);
  cout<<"epoll_fd is "<<epoll_fd_<<endl;
  epoll_event ev;
  ev.data.fd=ser_sock;
  ev.events=EPOLLIN;
  epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,ser_sock,&ev);
  for(;;)
  {
    std::vector<int> _fd;
    epoll_event event[100];
    _fd.clear();
    int i = epoll_wait(epoll_fd_,event,sizeof(event)/sizeof(event[0]),-1);
    for(int j=0;j<i;j++)
    {

      //当受到一个新客户端连接请求
      if(event[j].data.fd==ser_sock)
      {
        sockaddr_in cli_addr;
        socklen_t len=sizeof(cli_addr);
        epoll_event e;
        int cli_sock = accept(ser_sock,(sockaddr*)&cli_addr,&len);
        e.data.fd=cli_sock;
        e.events=EPOLLIN;
        epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,cli_sock,&e);
        continue;
      }

      //当是其中一个客户端普通请求
      else
      {
        char buf[10240];
        ssize_t s = read(event[j].data.fd,buf,10240);
        if(s==0)
        {
          epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,event[j].data.fd,NULL);
          cout<<"one client out!"<<endl;
        }
        else
        {
          cout<<buf<<endl;
          std::string rsp="HTTP/1.0 200 OK\r\n\r\n<html><h3>hallow new client!</h3></html>\r\n";
          write(event[j].data.fd,rsp.c_str(),rsp.length());
          epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,event[j].data.fd,NULL);
          close(event[j].data.fd);
          cout<<"over"<<endl;
        }
        continue;
      }
        continue;
    }
        continue;
  }
  close(epoll_fd_);
}

int main()
{
  EpollServer();
  
  return 0;
}
