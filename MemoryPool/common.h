#pragma once

#include<iostream>
#include<assert.h>
#include<vector>
#include<thread>
#include<mutex>
#include<map>

#ifdef _WIN32
#include<Windows.h>
#endif

//管理对象自由链表的长度
#define NLIST 240
//最大支持提供内存的大小
#define MAXBYTES 64*1024
#define NPAGES 129
#define PAGE_SHIFT 12

typedef size_t PageID;

//获得可以存放或取出一个指针的内存截断
static void*& GetAddr(void* list)		
{
	return *((void**)list);
}


class FreeList
{
public:
	bool IsEmpty()
	{
		if (list == nullptr)
			return true;
		return false;
	}

	void* Pop()
	{
		void* obj = list;
		list = GetAddr(list);
		size--;
		return obj;
	}

	void Push(void* obj)
	{
		GetAddr(obj) = list;
		list = obj;
		size++;
	}

	size_t Size()
	{
		return size;
	}

	size_t MaxSize()
	{
		return maxsize;
	}

	void* clear()
	{
		size = 0;
		void* _list = list;
		list = nullptr;
		return _list;
	}

	void PushRange(void* start, void* end, size_t num)
	{
		GetAddr(end) = list;
		list = start;
		size += num;
	}

	void SetMaxSize(size_t size)
	{
		maxsize = size;
	}

private:
	void* list = nullptr;
	size_t size = 0;
	size_t maxsize = 1;

};



//用于内存对齐，将不规范的内存大小转换成规范（规定）的大小
class Sizeclass
{
	//内存碎片控制在该内存大小的12%左右
	//[1,128]   8byte对齐  freelist[0,16)
	//[129,1024]  16byte对齐  freelist[16,72)
	//[1025,8*1024]		128byte对齐  freelist[72,128)
	//[8*1024+1,64*1024]	512byte对齐  freelist[128,240)
public:

	static size_t _RoundUp(size_t size, size_t align)
	{
		//先进位，再将对齐所对应的位置0，就实现了对齐位的倍数
		return (size + align - 1)&~(align - 1);
	}

	static size_t RoundUp(size_t bytes)
	{
		assert(bytes <= MAXBYTES);
		if (bytes <= 128)
		{
			return _RoundUp(bytes, 8);
		}
		else if (bytes <= 1024)
		{
			return _RoundUp(bytes, 16);
		}
		else if (bytes <= 8192)
		{
			return _RoundUp(bytes, 128);
		}
		else if (bytes <= 65536)
		{
			return _RoundUp(bytes, 512);
		}
		else
		{
			return -1;
		}
	}

	static size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	static size_t Index(size_t bytes)
	{
		assert(bytes < MAXBYTES);
		if (bytes <= 128)
		{
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024)
		{
			return _Index(bytes - 128, 4) + 16;
		}
		else if (bytes <= 8192)
		{
			return _Index(bytes - 1024, 7) + 16 + 56;
		}
		else if (bytes <= 65536)
		{
			return _Index(bytes - 8192, 9) + 16 + 56 + 56;

		}
		else
			return -1;
	}

	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = static_cast<int>(MAXBYTES / size);
		if (num < 2)
			num = 2;
		if (num > 512)
			num = 512;

		return num;
	}

	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= 12;
		if (npage == 0)
			npage = 1;
		return npage;
	}
};


struct Span
{
	PageID id;  //页号
	size_t npage;  //页的数量

	Span* next;
	Span* prev;

	void* list = nullptr;  //自由链表
	size_t objsize = 0;   //对象大小
	size_t usecount = 0;   //使用计数
};


class SpanList
{
public:
	SpanList()
	{
		head = new Span;
		head->next = head;
		head->prev = head;
	}

	Span* begin()
	{
		return head->next;
	}

	Span* end()
	{
		return head;
	}

	bool Empty()
	{
		return head->next == head;
	}

	void Insert(Span* cur,Span* newspan)
	{
		assert(cur);
		Span* prev = cur->prev;

		prev->next = newspan;
		newspan->prev = prev;
		newspan->next = cur;
		cur->prev = newspan;
	}

	void Erase(Span* cur)
	{
		assert(cur != nullptr && cur!= head);
		Span* prev = cur->prev;
		Span* next = cur->next;
		prev->next = next;
		next->prev = prev;
	}

	void PushFront(Span* cur)
	{
		Insert(begin(), cur);
	}

	Span* PopFront()
	{
		Span* span= begin();
		Erase(span);
		return span;
	}
	std::mutex mtx;
private:
	Span * head;
	
};