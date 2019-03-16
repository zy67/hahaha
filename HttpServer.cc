#include"HttpServer.h"
#include<signal.h>
HttpServer::HttpServer(std::string _port)
{
  port=atoi(_port.c_str());
}


bool HttpServer::TcpServerInit()
{
  _pool=new ThreadPool(10);
  _pool->ThreadPoolInit();
  struct sockaddr_in addr;
  addr.sin_family=AF_INET;
  //addr.sin_addr.s_addr=inet_addr(ip);
  addr.sin_addr.s_addr=INADDR_ANY;
  addr.sin_port=htons(port);
  ser_sock=socket(AF_INET,SOCK_STREAM,0);
  int opt=1;
  setsockopt(ser_sock,SOL_SOCKET,SO_REUSEADDR,(void*)&opt,sizeof(int));
  if(ser_sock<=0)
  {
    perror("socket");
    return 0;
  }
  if(bind(ser_sock,(struct sockaddr*)&addr,sizeof(addr))<0)
  {
    perror("bind");
    close(ser_sock);
    return 0;
  }
  listen(ser_sock,5);
  return 0;
}

bool HttpServer::Start()
{
  int cli_sock;
   sockaddr_in cli_addr;
  socklen_t cli_addr_len;
  while(1)
  {
    cli_sock=accept(ser_sock,(sockaddr*)&cli_addr,&cli_addr_len);
    if(cli_sock<0)
    {
      perror("accept");
      close(ser_sock);
      return 0;
    }
    HttpTask ht;
    ht.SetHttpTask(cli_sock,HttpHandler);
    _pool->PushTask(ht);
    std::cout<<"push over"<<std::endl;
    std::cout<<std::endl;
  }
  close(ser_sock);
}




int main(int argc,char* argv[])
{
  if(argc!=2)
  {
    std::cout<<"Please entry port"<<std::endl;
    return 0;
  }

  signal(SIGPIPE,SIG_IGN);
  HttpServer hs(argv[1]);
  hs.TcpServerInit();
  hs.Start();
  return 0;
}
