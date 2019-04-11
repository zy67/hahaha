#include<iostream>
#include<vector>

using std::endl;
using std::cout;
using std::vector;
using std::cin;


bool find(vector<vector<int> >& v,int _x,int _y,int n)
{
  int x=0;
  int y=0;
  while(x<_x && v[x][_y-1]<=n)
  {
    if(v[x][0]==n)
      return true;
    x++;
  }
  if(x==_x)
    return false;

  while(y<_y && v[_x-1][y]<=n)
  {
    if(v[x][y]==n)
      return true;
    y++;
  }
  if(y==_y)
    return false;

  int a=x,b=y;
  for(;a<_x;a++)
  {
    b=y;
    for(;b<_y;b++)
    {
      if(v[a][b]==n)
        return true;
    }
  }
  return false;
}





int main()
{
  int x,y;
  vector<int> _v;
  cout<<"please entry x: ";
  cin>>x;
  cout<<"please entry y: ";
  cin>>y;
  vector<vector<int> > v;
  int n=1;

  //初始化vector<vector<int> >     >>之间要有空格
  for(int i=0;i<x;i++)
  {
    _v.clear();
    for(int j=0;j<y;j++)
    {
      _v.push_back(n);
      n++;
    }
    v.push_back(_v);
  }
  int num;
  cout<<"please entry find: ";
  cin>>num;
  bool r=find(v,x,y,num);

  if(r==true)
    cout<<"true"<<endl;
  else
    cout<<"false"<<endl;
  return 0;
}
