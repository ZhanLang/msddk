#pragma once
#include <winternl.h>
#include <ntstatus.h>
#include <ulpc/winex.h>
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
		m_threadPool.Initialize(this, 0);
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
		CLIENT_CTXT* Ctxt = NULL;
		
		MYPORT_MESSAGE    PortMsg;
		memset(&PortMsg, 0, sizeof(PortMsg));
		while( TRUE )
		{
			st = ZwReplyWaitReceivePort(m_hPort,(LPVOID*)&Ctxt,NULL,&PortMsg);
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
					PCLIENT_MSG pCltMsg	= GET_MESSAGE_INFO_POINT( (LPVOID)Ctxt->View.ViewBase );
					PBYTE pMsgBody		= GET_MESSAGE_DATA_POINT( (LPVOID)Ctxt->View.ViewBase );

					if ( pCltMsg->nRetBufSize )
					{
						pOutMsg = malloc( pCltMsg->nRetBufSize );
						ZeroMemory(pOutMsg, pCltMsg->nRetBufSize);
					}
					
					int nRetSize = 0;
					pCltMsg->nRet = OnMsg(pCltMsg->nMsgID, pMsgBody, pCltMsg->nMsgSize, pOutMsg, pCltMsg->nRetBufSize, &nRetSize);
					
					pCltMsg->nMsgSize = nRetSize;
					SET_MESSAGE_INFO (pCltMsg, (LPVOID)Ctxt->View.ViewBase );
					if ( pCltMsg->nMsgSize && pOutMsg )
					{
						SET_MESSAGE_DATA( pOutMsg,pCltMsg->nMsgSize,(LPVOID)Ctxt->View.ViewBase);
					}

					if ( pOutMsg )
					{
						free(pOutMsg);
					}

					ZwReplyPort (Ctxt->ClientHandle, &PortMsg );
					ZwClose(Ctxt->ClientHandle);
					delete Ctxt;

					break;
				}
				else if ( msg_type == LPC_CONNECTION_REQUEST)
				{
					Ctxt = new CLIENT_CTXT;
					ZeroMemory(Ctxt, sizeof(CLIENT_CTXT));
					Ctxt->View.Length = sizeof(REMOTE_PORT_VIEW);

					NTSTATUS st = ZwAcceptConnectPort(&Ctxt->ClientHandle,Ctxt,&PortMsg,TRUE,NULL,&(Ctxt->View));
					if ( Ctxt->ClientHandle )
					{
						ZwCompleteConnectPort(Ctxt->ClientHandle);
					}
				}
				else if (msg_type == LPC_PORT_CLOSED )
				{
				}
			}
		}
	}

private:
	CThreadPoolEx<CThreadPoolWorker> m_threadPool;
	CString m_strPortName;
	HANDLE m_hPort;
};

