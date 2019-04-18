#pragma once
namespace msddk { ;

class CKeSpinLock : public NonPagedObject
{
private:
	KSPIN_LOCK m_SpinLock;
	KIRQL m_OwnerIrql;

#ifdef _DEBUG
	bool m_bHeld;
#endif

public:
	CKeSpinLock()
	{
		ASSERT(MmIsNonPagedSystemAddressValid(this));
		KeInitializeSpinLock(&m_SpinLock);
#ifdef _DEBUG
		m_bHeld = false;
#endif
	}

	void Lock()
	{
		KeAcquireSpinLock(&m_SpinLock, &m_OwnerIrql);
#ifdef _DEBUG
		ASSERT(!m_bHeld);
		m_bHeld = true;
#endif
	}

	void Unlock()
	{
#ifdef _DEBUG
		ASSERT(m_bHeld);
		m_bHeld = false;
#endif
		KeReleaseSpinLock(&m_SpinLock, m_OwnerIrql);
	}
};

};