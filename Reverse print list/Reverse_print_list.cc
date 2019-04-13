#include<iostream>

using std::cout;
using std::cin;
using std::endl;



struct ListNode
{
  int key;
  ListNode* next;
};


class List
{
  public:
    List()
    {
      head=new ListNode;
      head->key=0;
      head->next=NULL;
      last=head;
    }

    ~List()
    {
      ListNode* cur=head;
      while(head)
      {
        cur=head;
        head=cur->next;
        delete cur;
      }
      last=NULL;
    }
    
    void Add(int i)
    {
      ListNode* cur=new ListNode;
      cur->key=i;
      cur->next=NULL;
      last->next=cur;
      last=cur;
      return;
    }

    void Reverse_print()
    {
      List::Reverse(head->next);
    }

    void Print()
    {
      ListNode* cur=head->next;
      while(cur)
      {
        cout<<cur->key<<" ";
        cur=cur->next;
      }
      cout<<endl;
    }

    static void Reverse(ListNode* head)
    {
      if(head==NULL)
        return;
      Reverse(head->next);
      cout<<head->key<<" ";
      return;
    }

  private:
    //带头链表的头
    ListNode* head;
    ListNode* last;
};





int main()
{
  List list;
  for(int i=0;i<5;i++)
  {
    list.Add(i+1);
  }
  list.Print();
  list.Reverse_print();
  cout<<endl;
  return 0;
}
