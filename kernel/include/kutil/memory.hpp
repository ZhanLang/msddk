
#include "memory.h"

void * __cdecl operator new(size_t size)
{
	return msddk::npaged_new(size);
}

void __cdecl operator delete(void *ptr, size_t)
{
	msddk::npaged_delete(ptr);
}

 void * __cdecl operator new[](size_t size)
{
	 return msddk::npaged_new(size);
}

void __cdecl operator delete[](void *ptr)
{
	msddk::npaged_delete(ptr);
}

void __cdecl operator delete[](void *ptr, size_t)
{
	msddk::npaged_delete(ptr);
}