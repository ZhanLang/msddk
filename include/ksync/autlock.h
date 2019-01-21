#pragma once
namespace msddk { ;

template <class _T> 
class CKeFastLocker
{
private:
	_T * m_pLockable;

public:
	CKeFastLocker(_T &pLockable) : m_pLockable(&pLockable) 
	{ 
		if (m_pLockable) 
			m_pLockable->Lock(); 
	}
	~CKeFastLocker() 
	{ 
		if (m_pLockable) 
			m_pLockable->Unlock(); 
	}
};

template <class _T> 
class CKeFastUnlocker
{
private:
	_T * m_pLockable;

public:
	CKeFastUnlocker(_T &pLockable) : m_pLockable(&pLockable) 
	{ 
		if (m_pLockable) 
			m_pLockable->Unlock(); 
	}

	~CKeFastUnlocker() 
	{ 
		if (m_pLockable) 
			m_pLockable->Lock(); 
	}
};


template <class _T> 
class CKeReadLocker
{
private:
	_T * m_pLockable;
public:
	CKeReadLocker(_T &pLockable) : m_pLockable(&pLockable) 
	{ 
		if (m_pLockable) 
			m_pLockable->LockRead(); 
	}

	~CKeReadLocker() 
	{ 
		if (m_pLockable) 
			m_pLockable->UnlockRead(); 
	}
};

template <class _T> 
class CKeWriteLocker
{
private:
	_T * m_pLockable;

public:
	CKeWriteLocker(_T &pLockable) : m_pLockable(&pLockable) 
	{ 
		if (m_pLockable) 
			m_pLockable->LockWrite(); 
	}
	~CKeWriteLocker() 
	{ 
		if (m_pLockable) 
			m_pLockable->UnlockWrite(); 
	}
};


template <class _T> 
class CKeWriteFromReadLocker
{
private:
	_T * m_pLockable;

public:
	CKeWriteFromReadLocker(_T &pLockable) : m_pLockable(&pLockable) 
	{ 
		if (m_pLockable) 
		{ 
			m_pLockable->UnlockRead(); 
			m_pLockable->LockWrite();
		} 
	}
	~CKeWriteFromReadLocker() 
	{ 
		if (m_pLockable) 
		{ 
			m_pLockable->UnlockWrite();
			m_pLockable->LockRead(); 
		} 
	}
};
};