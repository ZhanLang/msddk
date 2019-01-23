#pragma once
#include <kutil/string.h>
#include "lpc.h"
#include <kutil/memory.h>
namespace msddk { ;

class CKeLpcClient {
public:
	CKeLpcClient(LPCWSTR lpszPortName) 
	{
		m_hSection = NULL;
		m_hPort = NULL;
		memset(&m_View, 0, sizeof(m_View));
		if ( lpszPortName )
			m_PortName = lpszPortName;
		

		if ( NT_SUCCESS(CreateSection()) )
		{
			Connect();
		}
	};
	~CKeLpcClient()
	{
		if ( m_hPort )
			ZwClose(m_hPort);
		
		if ( m_hSection )
			ZwClose(m_hSection);
	}

	NTSTATUS AskUser(int uCode, void *pInBuffer, ULONG InputLength, void * OutputBuffer, ULONG OutputLength)
	{
		UNREFERENCED_PARAMETER(uCode);
		UNREFERENCED_PARAMETER(pInBuffer);
		UNREFERENCED_PARAMETER(InputLength);
		UNREFERENCED_PARAMETER(OutputBuffer);
		UNREFERENCED_PARAMETER(OutputLength);
		size_t	total_size = 0;
		NTSTATUS st = STATUS_UNSUCCESSFUL;
		if ( !m_hPort )
			return STATUS_INVALID_HANDLE;
		
		total_size = sizeof(PORT_MESSAGE);
		ke_sentry<PPORT_MESSAGE, ke_default_sentry> pMsg = (PPORT_MESSAGE)npagednew(total_size);
		if ( !pMsg )
			return STATUS_NO_MEMORY;

		memset(pMsg, 0, total_size);

		pMsg->u1.s1.DataLength = (short)0;
		pMsg->u1.s1.TotalLength = (short)total_size;

		if ( !(m_View.ViewBase && m_View.ViewRemoteBase) )
		{
			return STATUS_INVALID_HANDLE;
		}

		CLIENT_MSG cmi = { STATUS_SUCCESS, 1, 0, 0 ,InputLength,0,uCode };
		SET_MESSAGE_INFO(&cmi, (VOID*)m_View.ViewBase);

		return st;
	}

private:
	NTSTATUS Connect()
	{
		NTSTATUS st = STATUS_UNSUCCESSFUL;

		ULONG			max_msglen = 0;
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