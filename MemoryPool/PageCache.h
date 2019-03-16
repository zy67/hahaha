#pragma once
#include"common.h"

class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &Inst;
	}

	Span* NewSpan(size_t npage);
	Span* _NewSpan(size_t npage);
	//获得从对象到span的映射
	Span* MapObjectToSpan(void* obj);
	//释放空间span回Pagecache，并合并相邻的span
	void ReleaseSpanToPageCache(Span* span);
private:
	SpanList pagelist[NPAGES];
private:
	static PageCache Inst;
	PageCache() = default;
	PageCache(const PageCache&) = delete;
	PageCache& operator=(const PageCache&) = delete;

	std::mutex _mtx;
	std::map<PageID, Span*> _id_span_map;
};

