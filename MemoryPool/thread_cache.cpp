#include"common.h"
#include"thread_cache.h"
#include"central_cache.h"

//向线程缓存申请内存
void* ThreadCache::Allocate(size_t size)
{
	assert(size < MAXBYTES);
	size_t _size = Sizeclass::RoundUp(size);
	size_t _index = Sizeclass::Index(size);
	if (this->_freelist[_index].IsEmpty())
	{
		return FetchFromCentralCache(_index, _size);
	}
	else
	{
		void* list = _freelist[_index].Pop();
		return list;
	}
}


//向线程缓存释放内存
void ThreadCache::Deallocate(void* ptr, size_t size)
{
	size_t _index = Sizeclass::Index(size);
	_freelist[_index].Push(ptr);

	//当自由链表对象超过一次批量申请的数量时
	//开始回收对象到中心缓存
	if (_freelist[_index].Size() >= _freelist[_index].MaxSize())
	{
		ListTooLong(&_freelist[_index], size);
	}
}

//线程缓存向中心缓存申请内存
void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	//根据所申请的对象的大小和要的频率获得申请的数量
	size_t num = min(Sizeclass::NumMoveSize(size),this->_freelist->MaxSize());
	void* start, *end;
	size_t fetchnum = CentralCache::GetInstance()->FetchRangeObj(start, end, num, size);
	if (fetchnum > 1)
	{
		_freelist[index].PushRange(GetAddr(start), end, fetchnum - 1);
	}
	if(num==_freelist->MaxSize())
	{
		_freelist->SetMaxSize(num + 1);
	}
	return start;

}


void ThreadCache::ListTooLong(FreeList* freelist, size_t size)
{
	void* start = freelist->clear();
	CentralCache::GetInstance()->ReleaseListToSpans(start, size);
}