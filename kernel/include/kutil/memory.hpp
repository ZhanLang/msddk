
#include "memory.h"


void * __cdecl operator new(size_t size)
{
	return ExAllocatePoolWithTag(NonPagedPool, size, 'WENB');
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