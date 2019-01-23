#pragma once
namespace msddk { ;
class CKeMutex
{
private:
	PKMUTEX m_pMutex;
public:
	CKeMutex()
	{
		m_pMutex = (PKMUTEX)npagednew(sizeof(KMUTEX));
		ASSERT(m_pMutex);
		if (m_pMutex)
			KeInitializeMutex(m_pMutex, 0);
	}

	~CKeMutex()
	{
		delete m_pMutex;
	}

	bool Valid()
	{
		return (m_pMutex != NULL);
	}

	void Lock()
	{
		LockEx();
	}

	bool TryLock()
	{
		return TryLockEx() == STATUS_SUCCESS;
	}

	void Unlock()
	{
		return Unlock(false);
	}

public:
	NTSTATUS LockEx(KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		return KeWaitForMutexObject(m_pMutex, WaitReason, WaitMode, Alertable, 0);
	}

	NTSTATUS TryLockEx(LONGLONG Timeout = 0,
		KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		LARGE_INTEGER liTimeout;
		liTimeout.QuadPart = Timeout;
		return KeWaitForMutexObject(m_pMutex, WaitReason, WaitMode, Alertable, &liTimeout);
	}

	void Unlock(bool NextCallIsWaitXXX)
	{
		ASSERT(Valid());
		KeReleaseMutex(m_pMutex, NextCallIsWaitXXX);
	}
};
};