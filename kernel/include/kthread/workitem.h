#pragma once
#include <kutil/types.h>
#include <ksync/event.h>
namespace msddk { ;

template<class T>
class CKeWorkItem
{
	typedef NTSTATUS (T::*P_INVOKER)( LPVOID lpParam);
public:
	CKeWorkItem(PDEVICE_OBJECT OB, T* pOwer, P_INVOKER Invoker)
	{
		m_pOwer = pOwer;
		m_pInvoker = Invoker;
		m_lpParam = NULL;
		m_pIoWorkItem = IoAllocateWorkItem(OB);

	}
	~CKeWorkItem()
	{
		IoFreeWorkItem(m_pIoWorkItem);
	}
public:
	NTSTATUS Exec(LPVOID lpParam)
	{
		m_lpParam = lpParam;
		IoQueueWorkItem(m_pIoWorkItem, 
			(PIO_WORKITEM_ROUTINE)WorkItemFunc,
			DelayedWorkQueue,
			this);
		m_event.Wait();
		return m_Ret;
	}

private:
	static VOID WorkItemFunc(PDEVICE_OBJECT  DeviceObject,PVOID  Context)
	{
		UNREFERENCED_PARAMETER(DeviceObject);
		CKeWorkItem* param = (CKeWorkItem*)Context;
		param->OnCallBack();
	}

	VOID OnCallBack()
	{
		m_Ret = (static_cast<T *>(m_pOwer)->*m_pInvoker)(m_lpParam);
		m_event.Set();
	}

private:
	CKeEvent m_event;
	void* m_pOwer;
	LPVOID m_lpParam;
	P_INVOKER m_pInvoker;
	PIO_WORKITEM m_pIoWorkItem;
	NTSTATUS m_Ret;
};


};