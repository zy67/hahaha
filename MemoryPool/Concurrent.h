#pragma once
#include"common.h"
#include"thread_cache.h"
#include"PageCache.h"
void* ConcurrentAlloc(size_t size)
{
	if (size > MAXBYTES)
	{
		int RoundSize = Sizeclass::_RoundUp(size, 1<<PAGE_SHIFT);
		int npage = RoundSize >> PAGE_SHIFT;
		Span* span = PageCache::GetInstance()->NewSpan(npage);
		return (void*)span->id;

		return malloc(size);
	}
	else
	{
		if (tls_threadcache == nullptr)
		{
			tls_threadcache = new ThreadCache;
		}
		return tls_threadcache->Allocate(size);
	}
}


void ConcurrentFree(void* ptr)
{
	Span* span = PageCache::GetInstance()->MapObjectToSpan(ptr);
	size_t size = span->objsize;
	if (size > MAXBYTES)
	{
		/*return free(ptr);*/
		PageCache::GetInstance()->ReleaseSpanToPageCache(span);
	}
	else
	{
		tls_threadcache->Deallocate(ptr, size);
	}
}

