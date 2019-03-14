#pragma once
#include "types.h"
void *	_cdecl operator new(size_t size);
void	__cdecl operator delete(void *ptr, size_t);
void *	__cdecl operator new[](size_t size);
void	__cdecl operator delete[](void *ptr);
void	__cdecl operator delete[](void *ptr, size_t);

namespace msddk { ;

static inline void *bulk_malloc(size_t size)
{
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
	KdPrint(("npagednew\n"));
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
}

static inline  void * _cdecl malloc(size_t size)
{
	return ExAllocatePoolWithTag(PagedPool, size, 'LACB');
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
