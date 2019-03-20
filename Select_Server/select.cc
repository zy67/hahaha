#include<iostream>
#include<stdlib.h>
#include<string>
#include<sys/socket.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

using std::cerr;
using std::endl;
using std::cout;

#define INIT -1;






int Strat(int port)
{
  int sock;
  sock=socket(AF_INET,SOCK_STREAM,0);
  if(sock<0)
  {
    cerr<<"socket error"<<endl;
    exit(0);
  }
  sockaddr_in ser_addr;
  ser_addr.sin_family=AF_INET;
  ser_addr.sin_port=htons(port);
  ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(sock,(sockaddr*)&ser_addr,sizeof(ser_addr)))
  {
    cerr<<"bind error"<<endl;
    exit(1);
  }
  if(listen(sock,5))
  {
    cerr<<"listen error"<<endl;
    exit(2);
  }
  cout<<"readly to work!"<<endl;
  return sock;
}



void InitFD_ADDR(int* addr)
{
  for(int i=0;i<1024;i++)
  {
    addr[i]=INIT;
  }
}


void InputAddr(int sock,int& size,int& max_fd,int* fd_addr,fd_set* readfds)
{
  FD_SET(sock,readfds);
  fd_addr[size]=sock;
  size++;
  for(int i=0;i<size;i++)
  {
    if(max_fd < fd_addr[i])
      max_fd=fd_addr[i];
  }
}


void DelAddr(int* fd_addr,int i,int& size,int& max_fd,fd_set* readfds)
{
  FD_CLR(fd_addr[i],readfds);
  fd_addr[i]=fd_addr[size-1];
  fd_addr[size-1]=INIT;
  size--;
  for(int i=0;i<size;i++)
  {
    if(max_fd < fd_addr[i])
      max_fd=fd_addr[i];
  }
}



void InitFds(int size,int* fd_addr,fd_set* readfds)
{
  for(int i=0;i<size;i++)
  {
    FD_SET(fd_addr[i],readfds);
  }
}



int main()
{
  int size=0;
  struct timeval time;
  time.tv_sec=10;
  time.tv_usec=0;
  int ser_sock=Strat(8888);
  int fd_addr[1024];
  InitFD_ADDR(fd_addr);
  fd_addr[0]=ser_sock;
  size++;
  int max_fd=ser_sock;
  for(;;)
  {
    int sock; 
    fd_set readfds;
    FD_ZERO(&readfds);
    InitFds(size,fd_addr,&readfds);
    int tmp=select(max_fd+1,&readfds,NULL,NULL,NULL);
    if(tmp>0)
    {

      for(int i=0;i<size;i++)
      {
        if(FD_ISSET(fd_addr[i],&readfds))
        {
          if(fd_addr[i]==ser_sock)
          {
            struct sockaddr_in cli_addr;
            socklen_t len;
            sock=accept(ser_sock,(sockaddr*)&cli_addr,&len);
            cout<<"have a new link"<<endl;
            InputAddr(sock,size,max_fd,fd_addr,&readfds);
            continue;
          }
          else
          {
            char buf[10240];
            ssize_t s=recv(fd_addr[i],buf,sizeof(buf),0);
            cout<<buf<<endl;
            std::string rsp="HTTP/1.0 200 OK\r\n\r\n<html><h3>hallow new client!</h3></html>\r\n";
            send(fd_addr[i],rsp.c_str(),rsp.length(),0);
            DelAddr(fd_addr,i,size,max_fd,&readfds);
            continue;
          }
        }
        continue;
      }
        
      continue;
    }

    else if(tmp==0)
    {
      cout<<"time out!"<<endl;
      continue;
    }
    
    else
    {
      cerr<<"select error!"<<endl;
      continue;
    }
 
  }

  return 0;
}
