///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlthread.h
/// @brief  Functional Template Library Base Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
/// @defgroup ftlthread ftl thread function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_THREAD_H
#define FTL_THREAD_H
#pragma once

#include <process.h>    //for _beginthreadex
#include "ftlFake.h"

namespace FTL
{
    #define FTL_MAX_THREAD_DEADLINE_CHECK   5000

    typedef enum tagFTLThreadWaitType
    {
        ftwtStop, 
        ftwtContinue,
        ftwtTimeOut,
        ftwtError,
    }FTLThreadWaitType;
    
	class CFLockObject
	{
	public:
		virtual BOOL Lock(DWORD dwTimeout = INFINITE) = 0;
		virtual BOOL UnLock() = 0;
	};

	FTLEXPORT class CFCriticalSection : public CFLockObject
	{
		//DISABLE_COPY_AND_ASSIGNMENT(CFCriticalSection);
	public:
		FTLINLINE CFCriticalSection();
		FTLINLINE ~CFCriticalSection();

		//һ��һ���߳̽���һ��CS�������ܹ�һ�ٵ��ظ������CS,����Ҫ��֤ ��Ӧ��UnLock
		FTLINLINE BOOL Lock(DWORD dwTimeout = INFINITE);
		FTLINLINE BOOL UnLock();
		FTLINLINE BOOL TryLock();
#ifdef FTL_DEBUG
		FTLINLINE BOOL IsLocked() const;
		FTLINLINE BOOL SetTrace(BOOL bTrace);
#endif
	private:
		CRITICAL_SECTION m_CritSec;
#ifdef FTL_DEBUG
		DWORD   m_currentOwner;     //ָ�����ĸ��߳������˸��ٽ���
		BOOL    m_fTrace;           //ָ���Ƿ�Ҫ��ӡ��־��Ϣ
		DWORD   m_lockCount;        //���ڸ����߳̽���ؼ�����εĴ���,�������ڵ���
#endif //FTL_DEBUG
	};

	CFCriticalSection::CFCriticalSection()
	{
#if (_WIN32_WINNT >= 0x0403)
		//ʹ�� InitializeCriticalSectionAndSpinCount �����������
		::InitializeCriticalSectionAndSpinCount(&m_CritSec,4000);
#else
		::InitializeCriticalSection(&m_CritSec);
#endif

#ifdef FTL_DEBUG
		m_lockCount = 0;
		m_currentOwner = 0;
		m_fTrace = FALSE;
#endif
	}

	CFCriticalSection::~CFCriticalSection()
	{
#ifdef FTL_DEBUG
		FTLASSERT(m_lockCount == 0); // ����ǰ������ȫ�ͷţ���������������
#endif
		::DeleteCriticalSection(&m_CritSec);
	}

	BOOL CFCriticalSection::Lock(DWORD dwTimeout/* = INFINITE*/)
	{
		UNREFERENCED_PARAMETER( dwTimeout );
		FTLASSERT(INFINITE == dwTimeout && ("CFCriticalSection NotSupport dwTimeOut"));
#ifdef FTL_DEBUG
		DWORD us = GetCurrentThreadId();
		DWORD currentOwner = m_currentOwner;
		if (currentOwner && (currentOwner != us)) // already owned, but not by us
		{
			if (m_fTrace) 
			{
				FTLTRACEEX(tlDetail,TEXT("Thread %d begin to wait for CriticalSection %x Owned by %d\n"),
					us, &m_CritSec, currentOwner);
			}
		}
#endif
		::EnterCriticalSection(&m_CritSec);

#ifdef FTL_DEBUG
		if (0 == m_lockCount++) // we now own it for the first time.  Set owner information
		{
			m_currentOwner = us;
			if (m_fTrace) 
			{
				FTLTRACEEX(tlDetail,TEXT("Thread %d now owns CriticalSection %x\n"), m_currentOwner, &m_CritSec);
			}
		}
#endif
		return TRUE;
	}

	BOOL CFCriticalSection::UnLock()
	{
#ifdef FTL_DEBUG
		DWORD us = GetCurrentThreadId();
		FTLASSERT( us == m_currentOwner ); //just the owner can unlock it
		FTLASSERT( m_lockCount > 0 );
		if ( 0 == --m_lockCount ) 
		{
			// begin to unlock
			if (m_fTrace) 
			{
				FTLTRACEEX(tlDetail,TEXT("Thread %d releasing CriticalSection %x\n"), m_currentOwner, &m_CritSec);
			}
			m_currentOwner = 0;
		}
#endif
		::LeaveCriticalSection(&m_CritSec);
		return TRUE;
	}

	BOOL CFCriticalSection::TryLock()
	{
		BOOL bRet = TryEnterCriticalSection(&m_CritSec);
		return bRet;
	}

#ifdef FTL_DEBUG
	BOOL CFCriticalSection::IsLocked() const
	{
		return (m_lockCount > 0);
	}
	BOOL CFCriticalSection::SetTrace(BOOL bTrace)
	{
		m_fTrace = bTrace;
		return TRUE;
	}
#endif
}

#endif //FTL_THREAD_H
