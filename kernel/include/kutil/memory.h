#pragma once
#include "types.h"
/*全局的new 和delete 都是从非分页内存上申请*/
void *	__cdecl operator new(size_t size);
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

static inline void bulk_free(void *Ptr, size_t)
{
	if (Ptr)
		ExFreePool(Ptr);
}

static inline void* npaged_new(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
}

static inline void* paged_new(size_t size)
{
	PAGED_CODE();
	return ExAllocatePoolWithTag(PagedPool, size, 'WENB');
}

static inline void npaged_delete(void *Ptr)
{
	PAGED_CODE();
	if ( Ptr )
		ExFreePoolWithTag(Ptr, 'WENB');
}

static inline void paged_delete(void *Ptr)
{
	if ( Ptr )
		ExFreePoolWithTag(Ptr, 'WENB');
}

static inline  void * _cdecl paged_malloc(size_t size)
{
	PAGED_CODE();
	return ExAllocatePoolWithTag(PagedPool, size, 'LACB');
}
static inline  void * _cdecl npaged_malloc(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'LACB');
}


static inline void _cdecl paged_free(void *Ptr)
{
	PAGED_CODE();
	if ( Ptr )
		ExFreePoolWithTag(Ptr, 'LACB');
}

static inline void _cdecl npaged_free(void *Ptr)
{
	if ( Ptr )
		ExFreePoolWithTag(Ptr, 'LACB');
}

class NonPagedObject
{
public:
	void *operator new(size_t size)
	{
		return npaged_new(size);
	}

	void operator delete(void *ptr, size_t)
	{
		npaged_delete(ptr);
	}
};

class PagedObject
{
public:
	void *operator new(size_t size)
	{
		PAGED_CODE();
		return paged_new(size);
	}

	void operator delete(void *ptr, size_t)
	{
		PAGED_CODE();
		paged_delete(ptr);
	}
};
};
