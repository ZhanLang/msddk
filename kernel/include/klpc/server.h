#pragma once
#include "lpc.h"
#include <kutil/memory.h>
#include <kutil/string.h>




namespace msddk { ;

class CKeLpcServer : public NonPagedObject
{
public:
	CKeLpcServer():m_ConntPort(NULL){}
	~CKeLpcServer()
	{
		if ( m_ConntPort )
		{
			ZwClose(m_ConntPort);
			m_ConntPort = NULL;
		}
	}
	NTSTATUS Create(CKeStringW strPort)
	{

		/*
		OBJECT_ATTRIBUTES	oa = { 0 };
		InitializeObjectAttributes(&oa,strPort,OBJ_CASE_INSENSITIVE,NULL,NULL);
		 ZwConnectPort(NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);

		
		NTSTATUS st = ZwCreatePort(&m_ConntPort,
			&oa,
			sizeof(PORT_MESSAGE),
			sizeof(PORT_MESSAGE),
			0);

		if ( !NT_SUCCESS(st) )
		{
			KdPrint(("CKeLpcServer::Create(): failed call to ZwCreatePort() (%wS)\n", MapNTStatus(st)));
			return st;
		}
		*/
		return STATUS_SUCCESS;
	}
	NTSTATUS Close();
private:
	HANDLE	m_ConntPort;
	//PORT_MESSAGE
};

};