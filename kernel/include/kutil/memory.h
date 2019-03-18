#pragma once
#include "types.h"
/*全局的new 和delete 都是从非分页内存上申请*/
void *	_cdecl operator new(size_t size);
void	__cdecl operator delete(void *ptr, size_t);
void *	__cdecl operator new[](size_t size);
void	__cdecl operator delete[](void *ptr);
void	__cdecl operator delete[](void *ptr, size_t);

namespace msddk { ;

static inline void *bulk_malloc(size_t size)
{
	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	return ExAllocatePool(PagedPool, size);
}

static inline void bulk_free(void *p, size_t)
{
	if (p)
	{
		ExFreePool(p);
	}
	else
	{
		KdPrint(("msddk::kmemory::bulk_free(null,0)\r"));
	}
}

static inline void* npagednew(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
}

static inline  void * _cdecl malloc(size_t size)
{
	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	return ExAllocatePoolWithTag(PagedPool, size, 'LACB');
}
static inline  void * _cdecl npagemalloc(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'LANB');
}


static inline void _cdecl free(void *p)
{
	if (p)
	{
		ExFreePoolWithTag(p, 'LACB');
	}
	else
	{
		KdPrint(("msddk::kmemory::free(null)\r"));
	}
}

class NonPagedObject
{
public:
	void *operator new(size_t size)
	{
		return npagednew(size);
	}
};

class PagedObject
{
public:
	void *operator new(size_t size)
	{
		return malloc(size);
	}
};
};
