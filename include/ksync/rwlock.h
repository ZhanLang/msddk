#pragma once
namespace msddk { ;

class CKeRWLock
{
private:
	ERESOURCE m_Resource;

public:
	CKeRWLock()
	{
		ExInitializeResourceLite(&m_Resource);
	}

	~CKeRWLock()
	{
		ExDeleteResourceLite(&m_Resource);
	}

	void LockRead()
	{
		ExAcquireResourceSharedLite(&m_Resource, TRUE);
	}

	void LockWrite()
	{
		ExAcquireResourceExclusiveLite(&m_Resource, TRUE);
	}

	void UnlockRead()
	{
		ExReleaseResourceLite(&m_Resource);
	}

	void UnlockWrite()
	{
		ExReleaseResourceLite(&m_Resource);
	}
};

};