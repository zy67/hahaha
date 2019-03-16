#include"HttpServer.h"

using std::string;

enum boundary_type
{
  BOUNDARY_NO=0,
  BOUNDARY_FIRST,
  BOUNDARY_MIDDLE,
  BOUNDARY_LAST,
  BOUNDARY_BAK     //BOUNDARY的一部分
};


class Upload
{
  private:
    int file_fd;
    string file_name;
    string first_boundary;
    string middle_boundary;
    string last_boundary;
    
    //匹配boundary字符串
    int CatchBoundary(char* buf,size_t _len,int* boundary_pos)
    {
      if(!memcmp(buf,first_boundary.c_str(),first_boundary.length()))
      {
        *boundary_pos=0;
        return BOUNDARY_FIRST;
      }
      for(size_t i=0;i<_len;i++)
      {
        if((_len-i)>=last_boundary.length())
        {

          if(!memcmp(buf+i,middle_boundary.c_str(),middle_boundary.length()))
          {
            *boundary_pos=i;
            return BOUNDARY_MIDDLE;
          }
          else if(!memcmp(buf+i,last_boundary.c_str(),last_boundary.length()))
          {
            *boundary_pos=i;
            return BOUNDARY_LAST;
          }
        }

        else 
        {
          int cmp_len=_len-i;
          if(!memcmp(buf+i,middle_boundary.c_str(),cmp_len))
          {
            *boundary_pos=i;
            return BOUNDARY_MIDDLE;
          }
          else if(!memcmp(buf+i,last_boundary.c_str(),cmp_len))
          {
            *boundary_pos=i;
            return BOUNDARY_LAST;
          }

        }
      }

      return BOUNDARY_NO;
    }


    bool GetFileName(char* buf,int* content_pos)
    {
      char* p=NULL;
      p=strstr(buf,"\r\n\r\n");
      if(p==NULL)
      {
        *content_pos=0;
        return false;
      }

      *content_pos=p-buf+4;
      string head;
      head.assign(buf,p-buf);
      string file_seq="filename=\"";
      size_t pos=head.find(file_seq);
      if(pos==string::npos)
        return false;
      string file;
      file=head.substr(pos+file_seq.length());
      pos=file.find("\"");
      file.erase(pos);
      file_name="WWW/"+file;

      return true;
    }


    bool WriteFile(char* buf,int len)
    {
      if(file_fd!=-1)
      { 
        write(file_fd,buf,len);
        return true;
      }
      return false;
    }

    bool CreateFile()
    {
      file_fd=open(file_name.c_str(),O_CREAT|O_WRONLY,0644);
      if(file_fd<0)
      {
        perror("open file");
        return false;
      }
      return true;
    }

    bool CloseFile()
    {
      if(file_fd!=-1)
      {
        close(file_fd);
        file_fd=-1;
      }
      return true;
    }




  public:
    Upload():file_fd(-1){}

    ~Upload()
    {
      close(file_fd);
    }

    bool InitUpload()
    {
      //获取boundary
      umask(0);
      char* p=getenv("Content-Type");
      if(p==NULL)
      {
        std::cerr<<"Content-Type error";
        return false;
      }
      string find_boundary="boundary=";
      string content_type=p;
      size_t pos=content_type.find(find_boundary);
      if(pos==string::npos)
      {
        std::cerr<<"not find boundary";
        return false;
      }
      string boundary=content_type.substr(pos+find_boundary.length());
      first_boundary="--"+boundary;
      middle_boundary="\r\n"+first_boundary+"\r\n";
      last_boundary="\r\n"+first_boundary+"--\r\n";
      return true;
    }

    bool ProcessUpload()
    {
      //len表示已读的长度，_len表示还有的长度。
      int64_t len=0,_len=0;
      string content_len=getenv("Content-Length");
      int64_t file_len=atoi(content_len.c_str());
      char buf[MAX_BUF]={0};
      
      while(len < file_len)
      {
        int rlen=read(0,buf+_len,MAX_BUF);
        len+=rlen;
        _len+=rlen;
        //boundary起始位置
        int boundary_pos=0;
        //文件起始位置
        int content_pos=0;
      

        int flag=this->CatchBoundary(buf,_len,&boundary_pos);
        if(flag == BOUNDARY_FIRST)
        {
          if(GetFileName(buf,&content_pos))
          {
            CreateFile();
            _len-=content_pos;
            //用后面的有效数据向前移至起始位置
            memmove(buf,buf+content_pos,_len);
          }
          else
          {
            _len-=first_boundary.length();
            memmove(buf,buf+first_boundary.length(),_len);
          }
        }


        while(1)
        {
          int flag=CatchBoundary(buf,_len,&boundary_pos);
          if(flag!=BOUNDARY_MIDDLE)
          {
            break;
          }
          WriteFile(buf,boundary_pos);
          CloseFile();
          _len-=boundary_pos;
          memmove(buf,buf+boundary_pos,_len);
          if(GetFileName(buf,&content_pos))
          {
            CreateFile();
            _len-=content_pos;
            memmove(buf,buf+content_pos,_len);
          }
          else
          {
            if(content_pos==0)
              break;
            _len-=middle_boundary.length();
            memmove(buf,buf+middle_boundary.length(),_len);
          }
        }


        int tmp1=CatchBoundary(buf,_len,&boundary_pos);
        if(tmp1==BOUNDARY_LAST)
        {
          WriteFile(buf,boundary_pos);
          CloseFile();
          return true;
        }
        int tmp2=CatchBoundary(buf,_len,&boundary_pos);
        if(tmp2==BOUNDARY_BAK)
        {
          WriteFile(buf,boundary_pos);
          _len-=boundary_pos;
          memmove(buf,buf+boundary_pos,_len);
        }
        int tmp3=CatchBoundary(buf,_len,&boundary_pos);
        if(tmp3==BOUNDARY_NO)
        {
          WriteFile(buf,_len);
          _len=0;
        }
      }

      return true;
    }
};





int main()
{

  std::cerr<<"hallow"<<std::endl;
  Upload up;
  string rsp_body;
  if(up.InitUpload()==false)
  {

    std::cerr<<"Init"<<std::endl;
    rsp_body="<html><body><h1>500<h1></body></html>\r\n";
    write(1,rsp_body.c_str(),rsp_body.length());
    return 1;
  }
  
  std::cerr<<"uploading !"<<std::endl;

  if(up.ProcessUpload()==false)
  {
    std::cerr<<"upload"<<std::endl;
    rsp_body="<html><body><h1>404<h1></body></html>\r\n";
    write(1,rsp_body.c_str(),rsp_body.length());
    return 1;
  }
  else
    rsp_body="<html><body><h1>SUCCESS<h1></body></html>\r\n";
  write(1,rsp_body.c_str(),rsp_body.length());
  return 0;
}




