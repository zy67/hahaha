#include<iostream>
#include<string>
#include<string.h>

using std::endl;
using std::cout;
using std::cin;
using std::string;


void Replace(char* str)
{
  int num=0;
  int i=0;
  int len=strlen(str);
  for(;i<len;i++)
  {
    if(str[i] == ' ')
      num++;
  }
  num=num*2;
  int newlen = len+num;
  while(len!=newlen)
  {
    if(str[len-1]!=' ')
    {
      str[newlen-1]=str[len-1];
      newlen--;
      len--;
      continue;
    }
    else
    {
      str[newlen-1]='0';
      newlen--;
      str[newlen-1]='2';
      newlen--;
      str[newlen-1]='%';
      newlen--;
      len--;
      continue;
    }
  }
}





int main()
{
  char str[100];
  cout<<"please entry a string:";
  //C/C++中cin和scanf以空格或回车作为空格符，当要在字符串中输入空格时，使用cin.getline()函数
  cin.getline(str,100);
  Replace(str);
  cout<<"after replace :"<<str<<endl;
  return 0;
}
