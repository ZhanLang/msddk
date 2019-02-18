#pragma once
#include <kutil/string.h>
#include <kutil/dbg.h>
namespace msddk { ;
class CKeLog
{
public:
	CKeLog(LPCWSTR lpszLogFile):m_hFile(NULL)
	{
		OBJECT_ATTRIBUTES objectAttributes;
		IO_STATUS_BLOCK iostatus;
		UNICODE_STRING UniName;
		RtlInitUnicodeString(&UniName, lpszLogFile);
		
		InitializeObjectAttributes(&objectAttributes, &UniName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		NTSTATUS st = ZwCreateFile(&m_hFile, FILE_APPEND_DATA, &objectAttributes,
			&iostatus, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ, FILE_OPEN_IF,
			FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

		if ( !NT_SUCCESS(st ))
		{
			KdPrint(("LogFile Create Failed{%ws,%ws}", lpszLogFile, MapNTStatus(st)));
		}
	}

	NTSTATUS Log(const char* log, ...)
	{
		if ( !m_hFile )
			return STATUS_INVALID_HANDLE;

		
		NTSTATUS st = STATUS_SUCCESS;
		va_list argList;
		va_start(argList, log);
		st = Log(log, argList);
		va_end(argList);

		return st;
	}

	NTSTATUS Log(const char*log, va_list args)
	{
		CKeStringA strLog;
		strLog.FormatV(log, args);
		return Log2(strLog);
	}

	NTSTATUS Log2(const char*log)
	{
		IO_STATUS_BLOCK iostatus;
		return ZwWriteFile(m_hFile, NULL, NULL, NULL, &iostatus, (void*)log, (ULONG)strlen(log), NULL, NULL);
	}

	~CKeLog()
	{
		if ( m_hFile )
		{
			ZwClose(m_hFile);
			m_hFile = NULL;
		}
	}

private:
	HANDLE m_hFile;
};
};
