#pragma once
#ifdef UFRAME_EXPORTS
#define UFRAME_API __declspec(dllexport)
#else
#define UFRAME_API __declspec(dllimport)
#pragma comment(lib,"uframe.lib")
#endif

typedef int (_stdcall *frame_cb)(void* param,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
extern "C"
{
	UFRAME_API void*	__cdecl frame_create(const wchar_t* drvName, void* param, frame_cb cb);
	UFRAME_API void		__cdecl frame_close(void* frame);
	UFRAME_API HRESULT	__cdecl frame_send(void* frame,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
	UFRAME_API HRESULT	__cdecl frame_send2(const wchar_t* drvName, int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
};