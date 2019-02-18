#pragma once

#include "worker_thread.h"
namespace msddk { ;

template<class T>
class CkeWorkerThreadEx : public CkeWorkerThread
{	
public:
	void EnqueueItem(T pData)
	{
		T* data = new T(pData);
		__super::EnqueueItem(data);
	}

protected:
	virtual void DispatchItem(const T& item) = 0;

private:
	void DispatchItem(void *pItem)
	{
		T* data = (T*)pItem;
		DispatchItem(*data);
		delete data;
	}
};

};