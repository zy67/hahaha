#include"PageCache.h"
PageCache PageCache::Inst;



Span* PageCache::NewSpan(size_t npage)
{
	//避免递归加锁
	//std::unique_lock<std::mutex> lock(_mtx);
	//该位置正好有内存
	_mtx.lock();
	Span* span = _NewSpan(npage);
	_mtx.unlock();
	return span;
}

Span* PageCache::_NewSpan(size_t npage)
{
	//若申请页数比NPAGES多，直接申请一个相应大小的span给它
	if (npage >= NPAGES)
	{
		void* ptr = VirtualAlloc(NULL, (npage) << PAGE_SHIFT, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		Span* newspan = new Span;
		newspan->id = (PageID)ptr >> PAGE_SHIFT;
		newspan->npage = npage;
		newspan->objsize = npage << PAGE_SHIFT;
		
		for (size_t i = 0; i < npage; i++)
		{
			_id_span_map[newspan->id + i] = newspan;
		}
		return newspan;
	}

	if (!pagelist[npage].Empty())
	{
		return pagelist->PopFront();
	}

	//向下遍历，找到有内存的位置，将内存进行重分配
	for (size_t i = npage + 1; i < NPAGES; i++)
	{
		if (!pagelist[i].Empty())
		{
			Span* span = pagelist[i].PopFront();
			Span* newspan = new Span;
			newspan->id = span->id + span->npage - npage;
			newspan->npage = npage;
			span->npage -= npage;
			for (size_t i = 0; i < newspan->npage; i++)
			{
				_id_span_map[newspan->id + i] = newspan;
			}
			pagelist[span->npage].PushFront(span);
			//std::cout << "fengehou" << span->id<<std::endl;

			return newspan;
		}
	}

	//没有空余内存需要向系统申请内存 
	void* ptr = VirtualAlloc(NULL, (NPAGES - 1) << PAGE_SHIFT, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	Span* largespan = new Span;
	largespan->id = (PageID)ptr >> PAGE_SHIFT;
	largespan->npage = NPAGES - 1;
	//建立映射关系
	for (size_t i = 0; i < NPAGES; i++)
	{
		_id_span_map[largespan->id + i] = largespan;
	}

	pagelist[NPAGES - 1].PushFront(largespan);
	//std::cout << "diyicihuoqu" << largespan->id<<std::endl;
	return _NewSpan(npage);
}


Span* PageCache::MapObjectToSpan(void* obj)
{
	//通过地址获得页号,一个页占4k
	PageID pageid = (PageID)obj >> PAGE_SHIFT;
	//通过页号找到span对象
	auto it = _id_span_map.find(pageid);
	assert(it != _id_span_map.end());

	return it->second;
}


void PageCache::ReleaseSpanToPageCache(Span* span)
{
	std::unique_lock<std::mutex> lock(_mtx);
	if (span->npage >= NPAGES)
	{
		void* ptr = (void*)(span->id << PAGE_SHIFT);
		VirtualFree(ptr,0,MEM_RELEASE);
		_id_span_map.erase(span->id);
		delete span;
		return;
	}


	//找到参数前一个span
	auto previt = _id_span_map.find(span->id - 1);
	while (previt != _id_span_map.end())
	{
		Span* prevspan = previt->second;
		//不是闲置的直接跳出
		if (prevspan->usecount != 0)
			break;

		//合并超出NPAGES页的span也进行忽略
		if (prevspan->npage + span->npage >= NPAGES)
			break;

		pagelist[prevspan->npage].Erase(prevspan);
		prevspan->npage += span->npage;
		delete span;
		span = prevspan;
		//找上一个span
		previt = _id_span_map.find(span->id - 1);
	}
	//找该span地址后面紧跟的span
	auto nextit = _id_span_map.find(span->id + span->npage);
	while (nextit != _id_span_map.end())
	{
		Span* nextspan = nextit->second;
		if (nextspan->usecount != 0)
			break;
		if (nextspan->npage + span->npage >= NPAGES)
			break;
		
		pagelist[nextspan->npage].Erase(nextspan);
		span->npage += nextspan->npage;
		delete nextspan;
		//找下一个span
		nextit = _id_span_map.find(span->id+span->npage);
	}
	//建立新的映射关系
	for (size_t i = 0; i < span->npage; ++i)
	{
		_id_span_map[span->id + i] = span;
	}
	//std::cout << "hebinghou" << span->id << std::endl;
	pagelist[span->npage].PushFront(span);
}

