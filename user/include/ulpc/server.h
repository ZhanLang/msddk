#pragma once
#include <winternl.h>
#include <ntstatus.h>
#include "winex.h"
#include <atlutil.h>
#include <functional>
#include <klpc/lpc.h>

extern "C"
{
	NTSTATUS
		NTAPI
		ZwCreateSection (
		__out PHANDLE SectionHandle,
		__in ACCESS_MASK DesiredAccess,
		__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
		__in_opt PLARGE_INTEGER MaximumSize,
		__in ULONG SectionPageProtection,
		__in ULONG AllocationAttributes,
		__in_opt HANDLE FileHandle
		);

	NTSTATUS
		NTAPI
		ZwClose (
		__in HANDLE Handle
		);
};
class CThreadPoolWorker
{
public:
	typedef std::function<void()>* RequestType ;
	virtual BOOL Initialize(void *pvParam){return TRUE;}
	virtual void Terminate(void* /*pvParam*/){}

	void Execute( RequestType dw, void *pvParam,OVERLAPPED *pOverlapped) throw()
	{
		auto func = (RequestType)dw;
		(*func)();
		delete func;
	}
};

template<class _Base>
class CThreadPoolEx: public CThreadPool<_Base>
{
public:
	BOOL QueueRequest(const std::function<void()> &f)
	{
		auto func = new std::function<void()>(f);
		return CThreadPool<_Base>::QueueRequest(func);
	}
};
class CLpcServer
{
public:
	CLpcServer(void){}
	virtual ~CLpcServer(void)
	{
		Close();
	}

public:
	HRESULT Create(LPCWSTR lpszPortName)
	{
		if ( !(lpszPortName && wcslen(lpszPortName)) )
			return E_INVALIDARG;

		m_strPortName = lpszPortName;
		m_threadPool.Initialize(this, 10);
		return CreatePort();
	}
	HRESULT Close()
	{
		if ( m_hPort )
		{
			ZwClose(m_hPort);
			m_hPort = NULL;
		}
		m_threadPool.Shutdown(-1);
		return S_OK;
	}
	virtual int OnMsg(int uCode, void *pInBuffer, int InputLength, void * OutputBuffer, int nOutCch, int* OutputLength)
	{
		return STATUS_SUCCESS;
	}
	
private:
	HRESULT CreatePort()
	{
		NTSTATUS			st = STATUS_UNSUCCESSFUL;
		UNICODE_STRING		us_name = {0};
		OBJECT_ATTRIBUTES	oa	= {0};
		SECURITY_DESCRIPTOR sd	= {0};
		HRESULT				hr  = S_OK;

		if ( !m_strPortName)
			return E_INVALIDARG;

		RtlInitUnicodeString(&us_name, m_strPortName);
		InitializeObjectAttributes( &oa,&us_name,OBJ_CASE_INSENSITIVE,NULL, NULL);
		st = ZwCreatePort( &m_hPort,&oa,sizeof(MYPORT_MESSAGE),sizeof(MYPORT_MESSAGE),0 );
		if( !NT_SUCCESS(st) )
		{
			return HRESULT_FROM_NT(st);
		}

		BOOL bRet = m_threadPool.QueueRequest([this]{Listen();});
		if ( !bRet )
		{
			return E_FAIL;
		}
		 
		return hr;
	}
	
private:
	VOID Listen()
	{
		NTSTATUS st = STATUS_SUCCESS;
		HANDLE hConnectPort = NULL;
		struct client_ctxt {
			HANDLE commu_port;
			REMOTE_PORT_VIEW view;
		} ctxt;


		ZeroMemory(&ctxt,sizeof(ctxt));
		ctxt.view.Length = sizeof( REMOTE_PORT_VIEW );
		
		MYPORT_MESSAGE    PortMsg;
		memset(&PortMsg, 0, sizeof(PortMsg));
		while( TRUE )
		{
			st = ZwReplyWaitReceivePort(m_hPort,(LPVOID*)&ctxt,NULL,&PortMsg);
			if ( st == STATUS_PORT_CLOSED )
				break;
			else if ( st == STATUS_INVALID_HANDLE)
				break;
			else if(st == STATUS_TIMEOUT)
				continue;
			else if ( st == STATUS_SUCCESS )
			{
				short msg_type = PortMsg.u2.s2.Type;
				if ( msg_type == LPC_REQUEST)
				{
					m_threadPool.QueueRequest([this]{Listen();});
					void* pOutMsg		= NULL;
					PCLIENT_MSG pCltMsg	= GET_MESSAGE_INFO_POINT( (LPVOID)ctxt.view.ViewBase );
					PBYTE pMsgBody		= GET_MESSAGE_DATA_POINT( (LPVOID)ctxt.view.ViewBase );

					if ( pCltMsg->nRetBufSize )
					{
						pOutMsg = malloc( pCltMsg->nRetBufSize );
						ZeroMemory(pOutMsg, pCltMsg->nRetBufSize);
					}
					
					int nRetSize = 0;
					pCltMsg->nRet = OnMsg(pCltMsg->nMsgID, pMsgBody, pCltMsg->nMsgSize, pOutMsg, pCltMsg->nRetBufSize, &nRetSize);
					
					pCltMsg->nMsgSize = nRetSize;
					SET_MESSAGE_INFO (pCltMsg, (LPVOID)ctxt.view.ViewBase );
					if ( pCltMsg->nMsgSize && pOutMsg )
					{
						SET_MESSAGE_DATA( pOutMsg,pCltMsg->nMsgSize,(LPVOID)ctxt.view.ViewBase);
					}

					if ( pOutMsg )
					{
						free(pOutMsg);
					}
					ZwReplyPort ( hConnectPort, &PortMsg );
					break;
				}
				else if ( msg_type == LPC_CONNECTION_REQUEST)
				{
					st = ZwAcceptConnectPort(&hConnectPort,&ctxt,&PortMsg,TRUE,NULL,&(ctxt.view));
					if ( st != STATUS_SUCCESS )
					{
						ZwAcceptConnectPort(&hConnectPort, &ctxt,&PortMsg, FALSE, 0, 0);
					}
					if ( hConnectPort )
					{
						ZwCompleteConnectPort(hConnectPort);
					}
				}
				else if (msg_type == LPC_PORT_CLOSED )
				{
				}
			}
		}

		
		if ( hConnectPort )
		{
			ZwClose(hConnectPort);
			hConnectPort = NULL;
		}
	}

private:
	CThreadPoolEx<CThreadPoolWorker> m_threadPool;
	CString m_strPortName;
	HANDLE m_hPort;
};

