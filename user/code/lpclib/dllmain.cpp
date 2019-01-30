// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "lpclib.h"
#include "lpchelp.h"
HMODULE g_hModule = NULL;
CLpcModuleHelp g_LpcModule;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern"C"
{
	LPCLIB_API  void* __cdecl lpc_server_create(const wchar_t* name, void* param, lpc_cb cb)
	{
		if ( !g_LpcModule.IsLoad() )
			g_LpcModule.LoadLibrary(g_hModule);

		return g_LpcModule.lpc_server_create(name, param, cb);
	}

	LPCLIB_API void  __cdecl lpc_server_close( void* pSvr)
	{
		if ( !g_LpcModule.IsLoad() )
			g_LpcModule.LoadLibrary(g_hModule);

		return g_LpcModule.lpc_server_close(pSvr);
	}
	LPCLIB_API int   __cdecl lpc_send(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize,int* nRet)
	{
		if ( !g_LpcModule.IsLoad() )
			g_LpcModule.LoadLibrary(g_hModule);

		return g_LpcModule.lpc_send(name, uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize, nRet);
	}
};


