#pragma once
namespace msddk { ;

class CKeSemaphore
{
private:
	PKSEMAPHORE m_pSemaphore;
public:
	CKeSemaphore(int InitialCount = 0, int Limit = 0x7FFFFFFF)
	{
		ASSERT(InitialCount >= 0);
		ASSERT(Limit > 0);
		m_pSemaphore = (PKSEMAPHORE)npagednew(sizeof(KSEMAPHORE));
		if (m_pSemaphore)
			KeInitializeSemaphore(m_pSemaphore, InitialCount, Limit);
	}

	~CKeSemaphore()
	{
		delete m_pSemaphore;
	}

	bool Valid()
	{
		return (m_pSemaphore != NULL);
	}

	void Wait()
	{
		WaitEx();
	}

	void Signal()
	{
		return SignalEx();
	}

	bool TryWait(unsigned timeoutInMsec = 0)
	{
		return WaitWithTimeoutEx(((LONGLONG)timeoutInMsec) * 10000) == STATUS_SUCCESS;
	}

public:
	NTSTATUS WaitEx(KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		return KeWaitForSingleObject(m_pSemaphore, WaitReason, WaitMode, Alertable, NULL);
	}

	NTSTATUS WaitWithTimeoutEx(LONGLONG Timeout = 0,
		KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		LARGE_INTEGER liTimeout;
		liTimeout.QuadPart = -Timeout;
		return KeWaitForSingleObject(m_pSemaphore, WaitReason, WaitMode, Alertable, &liTimeout);
	}

	void SignalEx(int Count = 1, KPRIORITY Increment = IO_NO_INCREMENT, bool NextCallIsWaitXXX = false)
	{
		ASSERT(Count > 0);
		ASSERT(Valid());
		KeReleaseSemaphore(m_pSemaphore, Increment, Count, NextCallIsWaitXXX);
	}
};

};