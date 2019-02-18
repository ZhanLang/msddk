#pragma once
#include <kthread/worker_threadex.h>
#include <ktime/time.h>
#include "log.h"

namespace msddk { ;
class CKeLogWorker : public CkeWorkerThreadEx<CKeStringA>
{
public:
	CKeLogWorker(LPCWSTR lpszLog)
	{
		m_ullID = 0;
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
		return Push(strLog);
	}

private:
	NTSTATUS Push(const char* log)
	{
		LONG64 ull = InterlockedIncrement64(&m_ullID);
		if (MAXULONG64 == ull)
		{
			InterlockedExchange64(&m_ullID, 1);
		}
		
		CKeStringA strLog;
		CKeStringA strTime = CKeTime().GetCurrentTimeString();
		strLog.Format("[%llu][%s] %s\r\n", ull, strTime.GetBuffer(), log);
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
	LONG64 m_ullID;
};
};

