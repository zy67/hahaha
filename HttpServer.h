#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#include<errno.h>
#include<error.h>
#include<vector>
#include<stdlib.h>
#include<sys/types.h>
#include<time.h>
#include<fcntl.h>
#include<iostream>
#include<pthread.h>
#include<string>
#include<queue>
#include<unordered_map>
#include<assert.h>
#include<sys/stat.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<limits.h>
#include<sstream> 
#include<dirent.h>
#include<sys/wait.h>



#define MAX_PATH 265
#define WWWROOT "WWW"
#define MAX_HTTPHD 4096
#define MAX_BUF 4096

class RequestInfo;
class HttpRequest;
class HttpResponse;
class Utils;




std::unordered_map<std::string,std::string> _err_desc={
  {"200","OK"},
  {"400","Bad Request"},
  {"403","Forbidden"},
  {"404","Not Found"},
  {"405","Method Not Allowed"},
  {"413","Request Entity Too Large"},
  {"500","Internal Server Error"}
};

std::unordered_map<std::string,std::string> g_mime_type={
  {"txt","text/plain"},
  {"html","text/html"},
  {"htm","text/html"},
  {"jpg","image/jpeg"},
  {"zip","application/zip"},
  {"mp3","audio/mpeg"},
  {"mpeg","video/mpeg"},
  {"unknow","application/octet-stream"}
};


typedef bool (*Handler)(int sock);



class Utils
{
public:
    static int Split(std::string& src,const std::string seg,std::vector<std::string>& list)
  {
    int num=0,idx=0;
    size_t pos;
    while(idx<src.length())
    {
      pos=src.find(seg,idx);
      if(pos==std::string::npos)
        break;
      list.push_back(src.substr(idx,pos-idx));
      num++;
      idx=pos+seg.length();
    }
    if(idx<src.length())
    {
      list.push_back(src.substr(idx));
      num++;
    }
    return num;
  }


    static std::string GetErrDesc(std::string & code)
    {
      auto it  =_err_desc.find(code);
      if(it!=_err_desc.end())
        return it->second;
        
      return "Unknow Error";
      
    }

    static void TimeToGmt(time_t t,std::string &gmt)
    {
      struct tm* mt= gmtime(&t);
      char tmp[128]={0};
      int len=strftime(tmp,127,"%a, %d %b %Y %H:%M:%S GMT",mt);
      gmt.assign(tmp,len);
    }

    static void DigitToStr(int64_t num,std::string& str)
    {
      std::stringstream ss;
      ss<<num;
      str=ss.str();
    }

    static void MakeETag(int64_t size,int64_t ino,std::string mtime,std::string &etag)
    {
      std::stringstream ss;
      ss<<"\"";
      ss<<std::hex<<ino;
      ss<<"-";
      ss<<std::hex<<size;
      ss<<"-";
      ss<<std::hex<<mtime;
      ss<<"\"";
      etag=ss.str();
    }

    static void GetMime(const std::string &file,std::string &mime)
    {
      size_t pos;
      pos=file.find_last_of(".");
      if(pos==std::string::npos)
      {
        mime=g_mime_type["unknow"];
        return;
      }
      std::string suffix=file.substr(pos+1);
      auto it=g_mime_type.find(suffix);
      if(it==g_mime_type.end())
      {
        mime=g_mime_type["unknow"];
      }
      else
      {
        mime=it->second;
      }
      return;
    }
};





class HttpTask
{
  
  private:
    //客户端socket成员
    int cli_sock;
    //http请求处理的任务
    Handler TaskHandler;

  public:
    void SetHttpTask(int sock,Handler handle)
    {
       cli_sock=sock;
       TaskHandler=handle;
    }
    
    void Handle()
    {
      TaskHandler(cli_sock);
    }

};


class ThreadPool
{
  private:
    int _max;
    int _num;
    bool stop=false;
    std::queue<HttpTask> _queue;
    pthread_mutex_t _lock;
    pthread_cond_t _cond;
  private:
    static void* start(void* arg)//成功获取线程，执行任务
    {
      pthread_detach(pthread_self());
      ThreadPool* p=(ThreadPool*) arg;
      while(1)
      {
        p->Lock();
        while(p->_queue.empty())
        {
          p->Wait();
          if(p->stop)
          {
            p->Unlock();
            p->_num--;
            pthread_exit(NULL);
          }        
        }
        if(p->stop)
        {
          p->Unlock();
          p->_num--;
          pthread_exit(NULL);
        }
        HttpTask ht=p->PopTask();
        p->Unlock();
        ht.Handle();         
      }
    } 
  public:
    ThreadPool(int max):_max(max),_num(0){}

    ~ThreadPool()
    {
      pthread_cond_destroy(&_cond);
      pthread_mutex_destroy(&_lock);
    }

    bool ThreadPoolInit()
    {
      while(_num<_max)
      {
          pthread_t tid;
          pthread_mutex_init(&_lock,NULL);
          pthread_cond_init(&_cond,NULL);
          pthread_create(&tid,NULL,ThreadPool::start,(void*)this);
          _num++;
      }
      return 1;
    }

    bool PushTask(HttpTask& ht)
    {
      Lock();
      if(stop)
      {
        Unlock();
        return 1;
      }
      _queue.push(ht);
      Unlock();
      WakeUpOne();
      return 1;
    }

    HttpTask PopTask()
    {
      HttpTask ht=_queue.front();
      _queue.pop();
      return ht;
    }

    bool ThreadPoolStop()
    {
      Lock();
      stop=true;
      Unlock();
      while(_num!=0)
      {
        WakeUpAll();
        usleep(2000);
      }
      return 1;
    }

    bool QueueEmpty()
    {
      return _queue.empty();
    }

    void Lock()
    {
      pthread_mutex_lock(&_lock);
    }

    void Unlock()
    {
      pthread_mutex_unlock(&_lock);
    }

    void Wait()
    {
      pthread_cond_wait(&_cond,&_lock);
    }

    void WakeUpOne()
    {
      pthread_cond_signal(&_cond);
    }

    void WakeUpAll()
    {
      pthread_cond_broadcast(&_cond);
    }

};



class RequestInfo    //包含HttpRequest解析出的请求信息
{
  public:
    std::string err;
    std::string _method;//请求方法
    std::string _version;//协议版本
    std::string _path_info;//资源路径
    std::string _path_phys;//资源实际路径
    std::string _query_string;//查询字符串
    std::unordered_map<std::string,std::string> _hdr_list;//整个头部信息中的键值对
    struct stat _st;
    bool IsCGI()
    {
      if((_method=="GET"&& !_query_string.empty())||(_method=="POST"))
        return true;
      return false;
    }
    
    void SetError(int error)
    {
      Utils::DigitToStr(error,err);
    }
};


class HttpRequest
{
  private:
    int cli_sock;
    std::string _http_header;
    RequestInfo _req_info;
  public:
    HttpRequest(int _cli_sock):cli_sock(_cli_sock){};

    bool RecvHttpHeader(RequestInfo& info)//接受http请求头
    {
      char buf[MAX_HTTPHD];
      while(1)
      {
        int ret=recv(cli_sock,buf,MAX_HTTPHD,MSG_PEEK);//接受请求头部，并不将接受缓存区的数据删掉
       // std::cout<<buf<<std::endl;
        if(ret<=0)
        {
          if(errno==EINTR||errno==EAGAIN)
            continue;
          info.SetError(500);
          return false;
        }
        char* pos=strstr(buf,"\r\n\r\n");
        if((pos==NULL)&&(ret==MAX_HTTPHD))  //头部太大，buf缓存区接受不下，报错
        {
          info.SetError(413);
          return false;
        }
        else if(pos==NULL&&ret<MAX_HTTPHD)  
        {
          continue;
        }
        int hand_len=pos-buf;
        _http_header.assign(buf,hand_len);//读len个字节
        recv(cli_sock,buf,hand_len+4,0);
        break;
      }
      std::cout<<_http_header<<std::endl;
      std::cout<<std::endl;
      return true;
    }

    bool ParseHttpHeader(RequestInfo& info)//解析http请求头
    {
      //http请求头解析
      std::vector<std::string> head_list;
      //将首部的每行信息分割，并存入head_list中
      Utils::Split(_http_header,"\r\n",head_list);
      ParseFirstLine(head_list[0],info);

      //将首部的键值对放入_hdr_list中
      for(int i=1;i<(head_list.size());i++)
      {
        size_t pos=head_list[i].find(": ");
        if(pos==std::string::npos)
          break;
        info._hdr_list[head_list[i].substr(0,pos)]=head_list[i].substr(pos+2);
      }
      return true;
    }
    
    //解析头部首行信息
    bool ParseFirstLine(std::string &line,RequestInfo &info)
    {
      std::vector<std::string> line_list;
      if(Utils::Split(line," ",line_list)!=3)
      {
          info.SetError(400);
          return false;
      }
      std::string url;
      info._method=line_list[0];
      url=line_list[1];
      info._version=line_list[2];

      if(info._method!="GET"&&info._method!="POST"&&info._method!="HEAD")
      {
        info.SetError(405);
        return false;
      }

      if(info._version!="HTTP/0.9"&&info._version!="HTTP/1.0"&&info._version!="HTTP/1.1")
      {
        info.SetError(500);
        return false;
      }

      size_t pos;
      pos=url.find('?');
      if(pos==std::string::npos)
      {
        info._path_info=url;
      }
      else{
        info._query_string=url.substr(pos+1);
        info._path_info=url.substr(0,pos);
      }

      if(IsLegalPath(info._path_info,info))
      {
        info.SetError(404);
        return false;
      }
      return true;
    }

    bool IsLegalPath(std::string &path,RequestInfo& info)
    {
      std::string file=WWWROOT+path;
      if(stat(file.c_str(),&info._st)<0)
      {
        info.SetError(404);
        return false;
      }
      char tmp[MAX_PATH]={0};
      realpath(file.c_str(),tmp);


      info._path_phys=tmp;
      if(info._path_phys.find(WWWROOT)==std::string::npos)
      {
        info.SetError(403);
        return false;
      }
      return true;
    }

    RequestInfo& GetRequestInfo()//获取解析结果
    {
      return _req_info;
    }
};


class HttpResponse
{
  private:
    int cli_sock;
    std::string _etag;//文件属性及最后一次修改时间
    std::string _mtime;//文件最后一次修改时间
    std::string _cont_len;//传输body的大小
    std::string _date;//系统响应时间
    std::string _fsize;//文件大小
    std::string _mime;//文件传输类型
  public:
    HttpResponse(int sock):cli_sock(sock){}

    bool FileIsDir(RequestInfo& info)
    {
      //判断路径是否为目录
      if(info._st.st_mode & S_IFDIR)  
      {
        if(info._path_phys.back()!='/')
          info._path_phys.push_back('/');
        if(info._path_info.back()!='/')
          info._path_info.push_back('/');
        return true;
      }
      return false;
    }

    bool ErrHandle(RequestInfo& info)
    {
        std::string rsp_body;
        rsp_body="<html><body><h1>"+info.err+"<h1></body></html>";
        int64_t _len=rsp_body.size();
        std::string _cont_len;
        Utils::DigitToStr(_len,_cont_len);

        std::string rsp_header;
        rsp_header=info._version+" "+info.err+" "+Utils::GetErrDesc(info.err)+"\r\n";
        rsp_header+="Content-Type: text/html;charset=UTF-8\r\n";
        rsp_header+="Content-len: "+_cont_len+"\r\n";
        rsp_header+="Transfer-Encoding: chunked\r\n";
        time_t t=time(NULL);
        std::string gmt;
        Utils::TimeToGmt(t,gmt);
        rsp_header+=gmt+"\r\n";
        SendCData(rsp_header);
        SendCData(rsp_body);
        return true;
    }

    bool InitResponse(RequestInfo &req_info)//初始化请求的响应信息
    {
      int64_t size=req_info._st.st_size;
      int64_t ino=req_info._st.st_ino;
      //将修改时间转换成GMT格式
      Utils::TimeToGmt(req_info._st.st_mtime,_mtime);
      //将数据放入_etag
      Utils::MakeETag(size,ino,_mtime,_etag);
      Utils::GetMime(req_info._path_phys,_mime);
      _fsize=req_info._st.st_size;
      time_t t=time(NULL);
      _date=t;
      return true;
    }
  
    bool SendData(std::string buf)
    {
      if(send(cli_sock,buf.c_str(),buf.size(),0)<0)
        return false;
      return true;
    }

    //chunke传输
    bool SendCData(const std::string &buf)
    {
      if(buf.empty())
      {
        return SendData("0\r\n\r\n");
      }
      std::stringstream ss;
      ss<<std::hex<<buf.length()<<"\r\n";
      SendData(ss.str());
      ss.clear();
      SendData(buf);
      SendData("\r\n");
      return true;
    }

    bool ProcessFile(RequestInfo &info)//文件下载处理
    {
      std::cout<<std::endl;
      std::cout<<"ProcessFile"<<std::endl;
      std::string rqs_head;
      rqs_head=info._version+" 200 OK\r\n";
      rqs_head+="Content-Type: "+_mime+"\r\n";
      rqs_head+="Content-Length: "+this->_fsize+"\r\n";
      rqs_head+="Etag: "+_etag+"\r\n";
      rqs_head+="Last-Modified: "+_mtime+"\r\n";
      rqs_head+="Date: "+_date+"\r\n";
      rqs_head+="Connextion: close\r\n\r\n";
      SendData(rqs_head);
      
      int fd=open(info._path_phys.c_str(),O_RDONLY);
      if(fd<0)
      {
        info.SetError(400);
        ErrHandle(info);
        return false;
      }
      int rlen=0;
      char tmp[MAX_BUF];
      while((rlen=read(fd,tmp,MAX_BUF))>0)
      {
        send(cli_sock,tmp,rlen,0);
      }
      close(fd);
      return true;
    }

    bool ProcessList(RequestInfo& info)//文件列表处理
    {
      //发送头部
      std::string rqs_head;
      rqs_head=info._version+" 200 OK\r\n";
      rqs_head+="Content-Type: text/html\r\n";
      rqs_head+="Etag: "+_etag+"\r\n";
      rqs_head+="Last-Modified: "+_mtime+"\r\n";
      rqs_head+="Date: "+_date+"\r\n";
      rqs_head+="Connextion: close\r\n\r\n";
      //if(info._version=="http/1.1")
      //  rqs_head+="Transfer-Encoding: chunked\r\n";
      //if(info._version=="http/1.1")
      //  SendCData(rqs_head);
      SendData(rqs_head);
      std::cout<<std::endl<<rqs_head; 

      //发送正文
      std::string rqs_body;
      rqs_body="<html><head><title>Index: /</title>";
      rqs_body+="<meta charset='UTF-8'></head>";
      rqs_body+="<body><h1>Index: "+info._path_info+"</h1>";
      rqs_body+="<from>";
      rqs_body+="<form action='/upload' method='POST' enctype='multipart/form-data'>";
      rqs_body+="<input type='file' name='UpLoadFile' value='abc' />";
      rqs_body+="<input type='submit' value='上传' />";
      rqs_body+="</form>";
      rqs_body+="<meta charset='UTF-8'></head>";
      rqs_body+="<hr /><ol>";
      SendData(rqs_body);
      std::cout<<rqs_body;
      FileIsDir(info);
      std::string path=info._path_phys;
      struct dirent **p_filename=NULL;
      int num=scandir(path.c_str(),&p_filename,0,alphasort);
      for(int i=0;i < num;i++)
      {
        std::string file_html="<li>";
        std::string name=p_filename[i]->d_name;
        std::string file=path+name;
        struct stat st;
        if(stat(file.c_str(),&st) < 0)
          continue;
        std::string mtime;
        std::string mime;
        std::string fsize;
        Utils::GetMime(name,mime);
        Utils::TimeToGmt(st.st_mtime,mtime);
        Utils::DigitToStr(st.st_size,fsize);
        file_html+="<strong><a href='"+info._path_info+name+"'>";
        file_html+=name+"</a></strong><br />";
        file_html+="<small>";
        file_html+="Modified:"+mtime+"<br />";
        file_html+=mime + "-"+fsize+"kbytes";
        file_html+="<br /><br /></small></li>";
        SendData(file_html);
        std::cout<<file_html<<std::endl;
      }
      std::string tail="</ol><hr /></body></html>\r\n\r\n";
      std::cout<<tail;
      std::cout<<std::endl;
      SendData(tail);
      //std::string end="";
      //SendData(end);
     
      return true;
    }

    bool ProcessCGI(RequestInfo& info)//CGI请求处理
    {
      int out[2],in[2];
      if(pipe(in) < 0)
      {
        info.SetError(500);
        perror("pipe error");
        return false;
      }
      //int out[2];
      if(pipe(out) < 0 )
      {
        info.SetError(500);
        perror("pipe error");
        return false;
      }
      pid_t pid=fork();
      if(pid < 0)
      {
        info.SetError(500);
        perror("fork error");
        return false;
      }
      else if(pid==0)
      {
        //子进程执行外部CGI程序，http头部用环境变量传递，正文用管道传递,CGI程序结果用管道传递
        setenv("METHOD",info._method.c_str(),1);
        setenv("VERSION",info._version.c_str(),1);
        setenv("PATH_INFO",info._path_info.c_str(),1);
        setenv("QUERY_STRING",info._query_string.c_str(),1);
        auto it=info._hdr_list.begin();
        while(it!=info._hdr_list.end())
        {
          setenv(it->first.c_str(),it->second.c_str(),1);
          it++;
        }
        close(in[1]);
        close(out[0]);
        std::cout<<info._path_phys<<std::endl;
        std::cout<<"int[0] = "<<in[0]<<" out[1] = "<<out[1]<<std::endl;
        dup2(in[0],0);
        dup2(out[1],1);
        std::cerr<<info._path_phys<<std::endl;
        if(execl(info._path_phys.c_str(),info._path_phys.c_str(),NULL)<0)
          std::cerr<<"execl worng!"<<std::endl;
        exit(0);        
      }
    
      close(in[0]);
      close(out[1]);
      //通过管道将正文数据传给子进程
      auto it=info._hdr_list.find("Content-Length");
      if(it!=info._hdr_list.end())
      {
        char buf[MAX_BUF]={0};
        int content_len=atoi(it->second.c_str());
        int rlen = 0;
        std::cout<<"正文"<<std::endl;
        while(rlen<content_len)
        {
          int len=MAX_BUF>(content_len-rlen)?(content_len-rlen):MAX_BUF;
          int read_len=recv(this->cli_sock,buf,len,0);
          std::cout<<buf;
          if(read_len<0)
          {
            std::cout<<"read woring!"<<std::endl;
            info.SetError(500);
            return false;
          }
          rlen+=read_len;
          if(write(in[1],buf,read_len)<0)
          {
            std::cout<<"write woring!"<<std::endl;
            info.SetError(500);
            return false;
          }

        }
      }
      //通过管道获得函数处理结果，并响应给客户端
      std::string rsp_head;
      rsp_head=info._method+" 200 OK\r\n";
      rsp_head+="Content-Type: text/html;charset=UTF-8\r\n";
      rsp_head+="Etag: "+_etag+"\r\n";
      rsp_head+="Date: "+_date+"\r\n";
      rsp_head+="Connextion: close\r\n\r\n";
      SendData(rsp_head);


      while(1)
      {
        char buf[MAX_BUF];
        int len=read(out[0],buf,MAX_BUF);
        if(len==0)
          break;
        send(cli_sock,buf,len,0);
      }

      close(in[1]);
      close(out[0]);
      waitpid(pid,NULL,0);
      return true;
    }
    
    bool FileHandler(RequestInfo &info)
    {
      InitResponse(info);
      if(FileIsDir(info))
      {
        if(ProcessList(info))
          return true;
        ErrHandle(info);
        return false;
      }
      else
      {
        if(ProcessFile(info))
          return true;
        ErrHandle(info);
        return false;
      }
    }

    bool CGIHandler(RequestInfo& info)
    {
      InitResponse(info);
      if(ProcessCGI(info))
        return true;
      ErrHandle(info);
      return false;
    }
};





class HttpServer
{
  private:
    int ser_sock;
    const char* ip;
    int port;
    ThreadPool* _pool;
  private:
    static bool HttpHandler(int sock)//http任务的处理函数
    {
      RequestInfo info;  //解析后的请求信息
      HttpRequest req(sock);//请求信息
      HttpResponse rsp(sock);//响应信息
      //接受请求头
      if(req.RecvHttpHeader(info)==false)
      {
        std::cout<<"err in RecvHttpHeader"<<std::endl;
        goto out;
      }
      //解析请求头
      if(req.ParseHttpHeader(info)==false)
      {
        std::cout<<"err in ParseHttpHeader"<<std::endl;
        goto out;
      }
      //判断请求类型
      if(info.IsCGI())
      {
        rsp.CGIHandler(info);
      }
      else
      {
        rsp.FileHandler(info);
      }

      close(sock);
      return true;


    out:
      rsp.ErrHandle(info);
      close(sock);
      return true;
    }
  public:
    HttpServer(std::string _port);
    bool TcpServerInit();//完成tcp服务端socket创建，完成线程池的初始化
    bool Start();//接受一个客户端连接请求，并创建任务，将任务压入队列
};
















#endif
