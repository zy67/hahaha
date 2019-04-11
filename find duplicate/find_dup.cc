#include<iostream>
#include<vector>
#include<unordered_map>
#include<map>
using std::cout;
using std::endl;
using std::cin;
using std::unordered_map;
using std::pair;

int find_hash(int n,std::vector<int>& v)
{
  unordered_map<int,int> hs;
  for(int i=0;i<n;i++)
  {
    if(v[i]<0||v[i]>=n)
      return -1;
    auto pos=hs.find(v[i]);
    if(pos==hs.end())
    {
      hs.insert(pair<int,int>(v[i],1));
      continue;
    }
    pos->second++;
  }
  auto it=hs.begin();
  while(it!=hs.end())
  {
    if(it->second!=1)
      return it->first;
    it++;
  }
  
  return -1;
}


//因为所有数都在0-n之内，可以将所有数放在下标对应的位置（合法下标），一定有位置满足条件的下标和其他合法下标是重复的，在比较时可发现重复。
int find_dup(int n,std::vector<int>& v)
{
  int i=0;
  for(;i<n;)
  {
    if(v[i]<0||v[i]>=n)
      return -1;
    if(v[i]==i)
    {
      i++;
      continue;
    }
    int index=v[i];
    if(v[index]==v[i])
      return v[i];
    int tmp=v[index];
    v[index]=v[i];
    v[i]=tmp;
  }
  return -1;
}





int main()
{
  int n;
  std::vector<int> v;
  cin>>n;
  int i=0;
  while(i<n)
  {
    int a;
    cin>>a;
    v.push_back(a);
    i++;
  }
  
  int num=find_dup(n,v);
  cout<<num<<endl;

  return 0;
}

