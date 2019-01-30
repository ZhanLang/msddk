
#pragma once

#ifdef LPCLIB_EXPORTS
#define LPCLIB_API __declspec(dllexport)
#else
#define LPCLIB_API __declspec(dllimport)
#pragma comment(lib,"lpclib.lib")
#endif

typedef int (_stdcall *lpc_cb)(void* param,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);

extern"C"
{
	LPCLIB_API void* __cdecl lpc_server_create(const wchar_t* name, void* param, lpc_cb cb);
	LPCLIB_API void  __cdecl lpc_server_close( void* pSvr);
	LPCLIB_API int   __cdecl lpc_send(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize,int* nRet);
};
