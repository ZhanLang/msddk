#pragma once
namespace msddk { ;

class CKeFastMutex
{
private:
	PFAST_MUTEX m_pMutex;

public:
	CKeFastMutex()
	{
		m_pMutex = (PFAST_MUTEX)npagednew(sizeof(FAST_MUTEX));
		if (m_pMutex)
			ExInitializeFastMutex(m_pMutex);
	}

	~CKeFastMutex()
	{
		delete m_pMutex;
	}

	bool Valid()
	{
		return (m_pMutex != NULL);
	}

	void Lock()
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
		ExAcquireFastMutex(m_pMutex);
	}

	bool TryLock()
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
		return ExTryToAcquireFastMutex(m_pMutex) != FALSE;
	}

	void Unlock()
	{
		ASSERT(Valid());
		ExReleaseFastMutex(m_pMutex);
	}
};

};