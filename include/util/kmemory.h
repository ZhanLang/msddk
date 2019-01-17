#pragma once
void * __cdecl operator new(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');;
}

void __cdecl operator delete(void *ptr, size_t)
{
	if (ptr)
	{
		ExFreePoolWithTag(ptr, 'WENB');
	}
}

void * __cdecl operator new[](size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
}

void __cdecl operator delete[](void *ptr)
{
	if (ptr)
	{
		ExFreePoolWithTag(ptr, 'WENB');
	}
	else
	{
		KdPrint(("msddk::kmemory::operator delete[](null)\n"));
	}
	
}

void __cdecl operator delete[](void *ptr, size_t)
{
	if (ptr)
		ExFreePoolWithTag(ptr, 'WENB');
	else
	{
		KdPrint(("msddk::kmemory::operator delete[](null,0)\n"));
	}
}

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
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
}

static  void * _cdecl malloc(size_t size)
{
	return ExAllocatePoolWithTag(PagedPool, size, 'LACB');
}

static void _cdecl free(void *p)
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
};
