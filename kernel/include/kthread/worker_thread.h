#pragma once
#include <kutil/memory.h>
#include <ksync/spinlock.h>
#include <ksync/semaphore.h>
#include <ksync/autlock.h>

#include "thread.h"
namespace msddk { ;

class CkeWorkerThread : public CKeThread, public NonPagedObject
{
private:
	CKeSpinLock m_SpinLock;
	CKeSemaphore m_Semaphore;

	struct QueueItem : public NonPagedObject
	{
		QueueItem *pNext;
		void *pParam;

		QueueItem(void *p)
			: pParam(p)
			, pNext(NULL)
		{
			ASSERT(MmIsNonPagedSystemAddressValid(this));
		}

	} *m_pHead, *m_pTail;

protected:
	virtual void DispatchItem(void *pItem) = 0;
	virtual void InitializeThread() {}
	virtual void FinalizeThread() {}

	CkeWorkerThread()
		: m_pHead(NULL)
		, m_pTail(NULL)
	{
		ASSERT(MmIsNonPagedSystemAddressValid(this));
	}

	~CkeWorkerThread()
	{
		if (IsRunning())
			Shutdown();

		for (QueueItem *pItem = m_pHead, *pNext; pItem; pItem = pNext)
		{
			pNext = pItem->pNext;
			delete pItem;
		}
	}

private:
	QueueItem * DoDequeueItem()
	{
		QueueItem *pItem = NULL;
		CKeFastLocker<CKeSpinLock> lck(m_SpinLock);
		pItem = m_pHead;
		if (!pItem)
			return NULL;

		//Dequeue item
		if (m_pTail == m_pHead)
		{
			ASSERT(m_pHead->pNext == NULL);
			m_pTail = NULL;
		}
		m_pHead = m_pHead->pNext;
		return pItem;
	}

private:
	virtual int ThreadBody() override
	{
		InitializeThread();

		for (;;)
		{
			m_Semaphore.Wait();
			QueueItem *pItem = DoDequeueItem();
			if (!pItem)
				continue;

			void *pParam = pItem->pParam;
			delete pItem;

			if (!pParam)
				break;

			DispatchItem(pParam);
		}

		FinalizeThread();
		return 0;
	}

public:
	void EnqueueItem(void *pData)
	{
		QueueItem *pItem = new QueueItem(pData);

		{
			CKeFastLocker<CKeSpinLock> lck(m_SpinLock);
			if (m_pTail == NULL)
				m_pHead = m_pTail = pItem;
			else
			{
				m_pTail->pNext = pItem;
				m_pTail = pItem;
			}
		}

		m_Semaphore.Signal();
	}

	void Shutdown(bool wait = true)
	{
		EnqueueItem(NULL);
		if (wait)
			Join();
	}

	void *TryGetItem()
	{
		QueueItem *pItem = DoDequeueItem();
		if (!pItem)
			return NULL;
		if (!pItem->pParam)
		{
			delete pItem;
			EnqueueItem(NULL);
			return NULL;
		}
		void *pResult = pItem->pParam;
		delete pItem;
		return pResult;
	}
};

};