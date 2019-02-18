#pragma once
#include <kutil/types.h>
extern "C" NTSYSAPI NTSTATUS NTAPI ZwWaitForSingleObject(
	__in HANDLE Handle,
	__in BOOLEAN Alertable,
	__in_opt PLARGE_INTEGER Timeout);

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS                ExitStatus;
	PVOID                   TebBaseAddress;
	CLIENT_ID               ClientId;
	KAFFINITY               AffinityMask;
	KPRIORITY               Priority;
	KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

extern "C" NTSTATUS NTAPI ZwQueryInformationThread(HANDLE ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID ThreadInformation,
	ULONG ThreadInformationLength,
	PULONG ReturnLength);

namespace msddk { ;
template <class DescendantClass>
class CKeBasicThread
{
private:
	HANDLE m_hThread;
	CLIENT_ID m_ID;

protected:
	typedef void _ThreadBodyReturnType;
	static inline _ThreadBodyReturnType _ReturnFromThread(void *pArg, ULONG retcode)
	{
		UNREFERENCED_PARAMETER(pArg);
		PsTerminateSystemThread((NTSTATUS)retcode);
	}

public:
	CKeBasicThread() : m_hThread(NULL)
	{
		m_ID.UniqueProcess = m_ID.UniqueThread = 0;
	}

	~CKeBasicThread()
	{
		Join();
		if (m_hThread)
			ZwClose(m_hThread);
	}

	BOOL Start(HANDLE hProcessToInject = NULL)
	{
		if (m_hThread)
			return FALSE;

		OBJECT_ATTRIBUTES threadAttr;
		InitializeObjectAttributes(&threadAttr, NULL, OBJ_KERNEL_HANDLE, 0, NULL);
		NTSTATUS st = PsCreateSystemThread(&m_hThread, 
			THREAD_ALL_ACCESS, 
			&threadAttr, 
			hProcessToInject, 
			&m_ID, DescendantClass::ThreadStarter, 
			this);

		if (!NT_SUCCESS(st))
			return FALSE;

		return TRUE;
	}

	BOOL Join()
	{
		if (!m_hThread)
			return TRUE;
		if (PsGetCurrentThreadId() == m_ID.UniqueThread)
			return FALSE;

		ZwWaitForSingleObject(m_hThread, FALSE, NULL);
		return TRUE;
	}

	BOOL Join(unsigned timeoutInMsec)
	{
		if (!m_hThread)
			return TRUE;

		if (PsGetCurrentThreadId() == m_ID.UniqueThread)
			return TRUE;

		LARGE_INTEGER interval;
		interval.QuadPart = (ULONGLONG)(-((LONGLONG)timeoutInMsec) * 10000);

		NTSTATUS st = ZwWaitForSingleObject(m_hThread, FALSE, &interval);
		return st == STATUS_SUCCESS;
	}

	BOOL IsRunning()
	{
		if (!m_hThread)
			return FALSE;

		LARGE_INTEGER zero = { 0, };
		return ZwWaitForSingleObject(m_hThread, FALSE, &zero) == STATUS_TIMEOUT;
	}

	VOID Reset()
	{
		Join();
		m_hThread = NULL;
		memset(&m_ID, 0, sizeof(m_ID));
	}

	INT GetReturnCode(BOOL *pbSuccess = NULL)
	{
		if (!m_hThread)
		{
			if (pbSuccess)
				*pbSuccess = FALSE;
			return -1;
		}
		THREAD_BASIC_INFORMATION info;
		NTSTATUS status = ZwQueryInformationThread(m_hThread, ThreadBasicInformation, &info, sizeof(info), NULL);
		if (!NT_SUCCESS(status))
		{
			if (pbSuccess)
				*pbSuccess = TRUE;
			return -1;
		}

		if (pbSuccess)
			*pbSuccess = TRUE;

		return info.ExitStatus;
	}

	DWORD GetThreadID()
	{
		return (DWORD)m_ID.UniqueThread;
	}

	static void Sleep(unsigned Millisecs)
	{
		LARGE_INTEGER interval;
		interval.QuadPart = (ULONGLONG)(-((LONGLONG)Millisecs) * 10000);
		KeDelayExecutionThread(KernelMode, FALSE, &interval);
	}

};
};