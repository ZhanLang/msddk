#pragma once

#include "basic_thread.h"
namespace msddk { ;

class CKeMemberThread :public CKeBasicThread<CKeMemberThread>
{
	typedef int (CKeMemberThread::*PMEMBER_INVOKER)();
	typedef CKeBasicThread<CKeMemberThread> _Base;
	void *m_pClass;

	PMEMBER_INVOKER m_pMemberInvoker;
	INT_PTR m_pThreadProc[2];
public:

	static inline _Base::_ThreadBodyReturnType __stdcall ThreadStarter(void *lp)
	{
		CKeMemberThread *pThread = ((CKeMemberThread *)((_Base *)lp));
		return _Base::_ReturnFromThread(lp, (pThread->*pThread->m_pMemberInvoker)());
	}

private:
	template <class _MemberClass, typename _MemberType> int MemberInvoker()
	{
		_MemberType pMember;
		C_ASSERT(sizeof(pMember) <= sizeof(m_pThreadProc));
		memcpy(&pMember, m_pThreadProc, sizeof(pMember));
		return (((_MemberClass *)m_pClass)->*pMember)();
	}

public:
	template <class _MemberClass, typename _MemberType> 
	MemberThread(_MemberClass *pClass, _MemberType pProc)
		: m_pClass(pClass)
	{
		ASSERT(pProc);
		C_ASSERT(sizeof(_MemberType) <= sizeof(m_pThreadProc));
		memcpy(m_pThreadProc, &pProc, sizeof(_MemberType));

		m_pMemberInvoker = &MemberThread::MemberInvoker<_MemberClass, _MemberType>;
	}
};

};