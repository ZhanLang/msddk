#pragma once
#include <kutil/memory.h>
#include <kutil/string.h>
#include "lpc.h"

namespace msddk { ;

class CKeLpcClient 
{
public:
	CKeLpcClient(LPCWSTR lpszPortName) 
	{
		m_hSection = NULL;
		m_hPort = NULL;
		memset(&m_View, 0, sizeof(m_View));
		if ( lpszPortName )
			m_PortName = lpszPortName;
	};

	~CKeLpcClient()
	{
		if (m_hPort)
		{
			ZwClose(m_hPort);
			m_hPort = NULL;
		}
		if (m_hSection)
		{
			ZwClose(m_hSection);
			m_hSection = NULL;
		}
	}

	NTSTATUS AskUser(int uCode, void *pInBuffer, int InputLength, void * OutputBuffer, int nOutCch, int* OutputLength)
	{
		NTSTATUS st = STATUS_UNSUCCESSFUL;
		MYPORT_MESSAGE Msg;
		KE_FAILED(CreateSection());
		KE_FAILED(Connect());

		memset(&Msg, 0, sizeof(Msg));
		Msg.u1.s1.DataLength = (short)0;
		Msg.u1.s1.TotalLength = (short)sizeof(Msg);

		if ( !(m_View.ViewBase && m_View.ViewRemoteBase) )
		{
			KdPrint(("CKeLpcClient::AskUser(): ViewBase is Null\n"));
			return STATUS_INVALID_HANDLE;
		}

		/*
		typedef struct _CLIENT_MESSAGE_INFO_
		{
			NTSTATUS	st_result;
			int			nMsgType;
			int			nCmd;
			int			nProd;
			int			nSubProd;
			int			nCPId;
			int			nCTId;
			int			nMsgSize;
			int			nRetBufSize;
			int			nRetSize;
			
		}CLIENT_MSG, *PCLIENT_MSG;
		*/

		CLIENT_MSG cmi = { STATUS_SUCCESS,0,uCode, 1, 0, 0, 0 ,(int)InputLength,nOutCch};
		SET_MESSAGE_INFO(&cmi, (VOID*)m_View.ViewBase);
		if (pInBuffer && InputLength)
		{
			SET_MESSAGE_DATA(pInBuffer, InputLength, m_View.ViewBase);
		}

		//阻塞线程等待服务端应答
		st = ZwRequestWaitReplyPort(m_hPort, &Msg, &Msg);
		if ( !NT_SUCCESS(st) )
		{
			KdPrint(("CKeLpcClient::AskUser(): failed call to ZwRequestWaitReplyPort() (%wS)\n", MapNTStatus(st)));
			return st;
		}

		if (OutputBuffer && OutputLength)
		{
			int			iMsgSize = GET_MESSAGE_DATA_SIZE(m_View.ViewBase);
			LPBYTE		pOutMsg  = GET_MESSAGE_DATA_POINT(m_View.ViewBase);
			*OutputLength = iMsgSize;

			if (iMsgSize)
			{
				memcpy(OutputBuffer, pOutMsg, iMsgSize);
			}
		}
		return st;
	}

private:
	NTSTATUS Connect()
	{
		NTSTATUS st = STATUS_UNSUCCESSFUL;
		ULONG	max_msglen = 0;

		m_View.Length = sizeof(PORT_VIEW);
		m_View.SectionHandle = m_hSection;
		m_View.ViewSize = LARGE_MESSAGE_SIZE;

		SECURITY_QUALITY_OF_SERVICE sqos = { 0 };
		sqos.Length = sizeof(sqos);
		sqos.ImpersonationLevel = SecurityImpersonation;
		sqos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
		sqos.EffectiveOnly = TRUE;

		st = ZwConnectPort(&m_hPort,m_PortName,&sqos,&m_View,NULL,&max_msglen,NULL,NULL);
		if (!NT_SUCCESS(st))
		{
			KdPrint(("CKeLpcClient::Connect(): failed call to ZwConnectPort() (%wS)\n", MapNTStatus(st)));
			return st;
		}

		return st;
	}

	NTSTATUS CreateSection()
	{
		NTSTATUS st = STATUS_UNSUCCESSFUL;
		LARGE_INTEGER offset = { LARGE_MESSAGE_SIZE };
		st = ZwCreateSection(&m_hSection, SECTION_ALL_ACCESS, NULL, &offset, PAGE_READWRITE, SEC_COMMIT, NULL);
		if (!NT_SUCCESS(st))
		{
			KdPrint(("CKeLpcClient::CreateSection(): failed call to ZwCreateSection() (%wS)\n", MapNTStatus(st)));
			return st;
		}

		return st;
	}
private:
	CKeStringW m_PortName;
	HANDLE m_hSection;
	HANDLE m_hPort;
	PORT_VIEW m_View;
};

};