#pragma once
#include"common.h"

class ThreadCache
{
public:
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);


	//从中心缓存获取所需对象
	void* FetchFromCentralCache(size_t index, size_t size);
	//链表中对象太多，开始回收内存
	void ListTooLong(FreeList* freelist,size_t size);

private:
	FreeList _freelist[NLIST];
};

//使用单件机制，告诉编译器此变量为线程内部使用，每个线程都copy一份，避免线程安全和效率问题
static _declspec(thread) ThreadCache* tls_threadcache = nullptr;