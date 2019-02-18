#pragma once
#include <kthread/worker_threadex.h>
#include "log.h"

namespace msddk { ;
class CKeLogWorker : public CkeWorkerThreadEx<CKeStringA>
{
public:
	CKeLogWorker(LPCWSTR lpszLog)
	{
		m_strLogFile = lpszLog;
		Start();
	}

	NTSTATUS PushLog(const char* log, ...)
	{
		CKeStringA strLog;
		va_list argList;
		va_start(argList, log);
		strLog.FormatV(log, argList);
		va_end(argList);

		EnqueueItem(strLog);

		return STATUS_SUCCESS;
	}

protected:
	virtual void DispatchItem(const CKeStringA& item)
	{
		KdPrint(("%s",(const char*)item));
		NTSTATUS st = CKeLog(m_strLogFile).Log(item);
		if ( !NT_SUCCESS(st) )
		{
			DbgPrint("logworker log faild {%ws},{%ws}", m_strLogFile.GetBuffer(), MapNTStatus(st));
			Sleep(1000);
			EnqueueItem(item);
		}
	}

	CKeStringW m_strLogFile;
};
};

