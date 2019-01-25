#pragma once
#include "basic_thread.h"
namespace msddk { ;

class CKeThread :public CKeBasicThread<CKeThread>
{
public:
	typedef CKeBasicThread<CKeThread> _Base;
	static typename _Base::_ThreadBodyReturnType __stdcall ThreadStarter(void *lp)
	{
		ASSERT(lp);
		CKeThread *pThread = ((CKeThread*)((_Base*)lp));
		int result = pThread->ThreadBody();
		return _Base::_ReturnFromThread(lp, result);
	}

protected:
	virtual int ThreadBody() = 0;
	virtual ~CKeThread() {}

public:
	CKeThread() {}
};

};