// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <ulpc/client.h>
#include <ulpc/server.h>
#include <ulpc/lpc.h>

class CExpServer : public CLpcServer
{
public:
	CExpServer( void* param, lpc_cb cb)
	{
		m_param = param;
		m_cb = cb;
	}
	virtual int OnMsg(int uCode, void *pInBuffer, int InputLength, void * OutputBuffer, int nOutCch, int* OutputLength)
	{
		if ( m_cb )
			return m_cb(m_param,uCode,pInBuffer,InputLength,OutputBuffer,nOutCch,OutputLength );
		return STATUS_NO_CALLBACK_ACTIVE;
	}
private:
	void* m_param;
	lpc_cb m_cb;
};


extern "C"
{
	void* _stdcall lpc_server_create(const wchar_t* name, void* param, lpc_cb cb)
	{
		if ( !cb )
			return NULL;

		CExpServer* pSvr = new CExpServer(param, cb);
		if ( pSvr )
		{
			if ( pSvr->Init(name) != S_OK)
			{
				delete pSvr;
				pSvr = NULL;
			}
		}

		return pSvr;
	}
	void  _stdcall lpc_server_close( void* srv)
	{
		if ( srv )
		{
			CExpServer* pSvr = (CExpServer*)srv;
			delete srv;
		}
	}

	int   _stdcall lpc_send(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize,int* nRet)
	{
		if ( !name )
			return STATUS_INVALID_PARAMETER;
		
		return CLpcClient(name).AskUser(uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize,nRet);
	}
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

