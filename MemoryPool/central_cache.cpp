#include"central_cache.h"
#include"common.h"
#include"PageCache.h"

CentralCache CentralCache::Inst;

size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t n, size_t byte)
{
	size_t index = Sizeclass::Index(byte);
	SpanList* _spanlist = &spanlist[index];
	Span* span = GetOneSpan(_spanlist, byte);

	void* cur = span->list;
	void* prev = cur;
	size_t fetchnum = 0;
	while (cur != nullptr&&fetchnum < n)
	{
		prev = cur;
		cur = GetAddr(cur);
		++fetchnum;
	}

	start = span->list;
	end = prev;
	GetAddr(end) = nullptr;

	span->list = cur;
	span->usecount += fetchnum;

	return fetchnum;
}

void CentralCache::ReleaseListToSpans(void* start, size_t byte_size)
{
	size_t index = Sizeclass::Index(byte_size);
	SpanList* _spanlist = &spanlist[index];
	//加锁
	std::unique_lock<std::mutex> lock(_spanlist->mtx);

	while (start)
	{
		void* next = GetAddr(start);
		Span* span = PageCache::GetInstance()->MapObjectToSpan(start);
		GetAddr(start) = span->list;
		span->list = start;
		//usecount==0时，代表该span切除的对象已被全部回收，可释放span到pagecache里进行合并
		if (--(span->usecount) == 0)
		{
			_spanlist->Erase(span);

			span->list = nullptr;
			span->objsize = 0;
			span->prev = nullptr;
			span->next = nullptr;

			PageCache::GetInstance()->ReleaseSpanToPageCache(span);
		}
		start = next;
	}

}


Span* CentralCache::GetOneSpan(SpanList* spanlist, size_t size)
{
	Span* span = spanlist->begin();
	while (span!=spanlist->end())
	{
		if (span->list != nullptr)
		{
			return span;
		}
		span = span->next;
	}

	//向pagecache申请一个合适的span
	size_t npage = Sizeclass::NumMovePage(size);
	Span* newspan = PageCache::GetInstance()->NewSpan(npage);

	//将span内存分割成一个个byte大小的对象挂起来
	char* start = (char*)(newspan->id << PAGE_SHIFT);//在pagecache中定义的id为实际地址>>PAGE_SHIFT
	char* end = start + (newspan->npage << PAGE_SHIFT);
	char* cur = start;
	char* next = cur + size;
	while (next < end)
	{
		GetAddr(cur) = next;
		cur = next;
		next = cur + size;
	}

	GetAddr(cur) = nullptr;
	newspan->list = start;
	newspan->objsize = size;
	newspan->usecount = 0;
	//将newspan插入spanlist

	spanlist->PushFront(newspan);
	return newspan;

}
