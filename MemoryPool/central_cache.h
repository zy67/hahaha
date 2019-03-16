#pragma once
#include"common.h"

//考虑线程安全，采用单例模式
class CentralCache
{
public:
	static CentralCache* GetInstance()
	{
		return &Inst;
	}
	//从中心缓存取出一定量内存给线程缓存
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte);
	
	//将一定数量内存释放到span跨度
	void ReleaseListToSpans(void* strat, size_t byte_size);

	Span* GetOneSpan(SpanList* spanlist, size_t size);
private:
	SpanList spanlist[NLIST]; //

private:
	static CentralCache Inst;

	CentralCache() = default;
	CentralCache(const CentralCache&) = delete;
	CentralCache& operator= (const CentralCache&) = delete;
};
