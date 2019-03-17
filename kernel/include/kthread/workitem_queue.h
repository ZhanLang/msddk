#pragma once
#include <kutil/types.h>
#include <ksync/event.h>
#include <kthread/basic_thread.h>
namespace msddk { ;

class CKeWorkItemQueue
{
public:
	struct WORKITEM_PARAM
	{
		LPVOID Data;
		CKeWorkItemQueue* pThis;
	};

	CKeWorkItemQueue()
	{
		m_Count = 0;
	}
public:
	virtual void DispatchItem(void *pItem) = 0;
	void EnqueueItem(void *pData)
	{
		WORKITEM_PARAM* Param = new WORKITEM_PARAM;
		Param->pThis = this;
		Param->Data = pData;
		PIO_WORKITEM pIoWorkItem = IoAllocateWorkItem(m_pDeviceObject);
		IoQueueWorkItemEx(pIoWorkItem, WorkItemEx, DelayedWorkQueue, Param);
	}

	void Initialize(PDEVICE_OBJECT pDev)
	{
		m_pDeviceObject = pDev;
	}

	void Shutdown(bool wait = true)
	{
		//方法有点low目前就这么处理吧
		while (wait && m_Count)
		{
			Sleep(1000);
		}
	}
public:
	static VOID WorkItemEx(
		_In_     PVOID        IoObject,
		_In_opt_ PVOID        Context,
		_In_     PIO_WORKITEM IoWorkItem
	)
	{
		UNREFERENCED_PARAMETER(IoObject);
		WORKITEM_PARAM* Param = (WORKITEM_PARAM*)Context;
		CKeWorkItemQueue* pThis = Param->pThis;
		LONG count = InterlockedIncrement(&pThis->m_Count);
		KdPrint(("+FlowContext->g_FlowCount:%d\n", count));
		Param->pThis->Do(Param->Data);
		count = InterlockedDecrement(&pThis->m_Count);
		KdPrint(("-FlowContext->g_FlowCount:%d\n", count));
		IoFreeWorkItem(IoWorkItem);
		delete Param;
	}
private:
	VOID Do( LPVOID lpParam)
	{
		UNREFERENCED_PARAMETER(lpParam);
		DispatchItem(lpParam);
	}

private:
	LONG m_Count;
	PDEVICE_OBJECT m_pDeviceObject;
};


};