#pragma once
#include "basic_thread.h"
namespace msddk { ; 

class CKeFunctionThread :public CKeBasicThread<CKeFunctionThread>
{
	typedef int(*PTHREADPROC)(void *lpParam);
	typedef  CKeBasicThread<CKeFunctionThread> _Base;

	PTHREADPROC m_pProc;
	void *m_lpParam;

public:
	static inline _Base::_ThreadBodyReturnType __stdcall ThreadStarter(void *lp)
	{
		CKeFunctionThread *pThread = ((CKeFunctionThread *)((_Base*)lp));
		ASSERT(pThread && pThread->m_pProc);
		return _Base::_ReturnFromThread(lp, pThread->m_pProc(pThread->m_lpParam));
	}

public:
	CKeFunctionThread(PTHREADPROC pProc, void *lpParam = NULL) :
		m_pProc(pProc),
		m_lpParam(lpParam)
	{
		ASSERT(pProc);
	}

};

};