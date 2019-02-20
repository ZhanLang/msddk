#pragma once
#include <kutil/types.h>
#include <ksync/event.h>
namespace msddk { ;

template<class T>
class CKeWorkItem
{
	typedef NTSTATUS (T::*P_INVOKER)( LPVOID lpParam);
public:
	CKeWorkItem(PDEVICE_OBJECT pDeviceObject, T* pOwer, P_INVOKER Invoker)
	{
		m_pOwer = pOwer;
		m_pInvoker = Invoker;
		m_lpParam = NULL;
		m_pDeviceObject = pDeviceObject;
	}
	~CKeWorkItem()
	{
	}
public:
	NTSTATUS Exec(LPVOID lpParam)
	{
		m_lpParam = lpParam;
		PIO_WORKITEM pIoWorkItem = IoAllocateWorkItem(m_pDeviceObject);
		IoQueueWorkItem(pIoWorkItem,WorkItemFunc,DelayedWorkQueue,this);
		m_event.Wait();
		IoFreeWorkItem(pIoWorkItem);
		return m_st;
	}

private:
	static VOID WorkItemFunc(PDEVICE_OBJECT  DeviceObject,PVOID  Context)
	{
		UNREFERENCED_PARAMETER(DeviceObject);
		((CKeWorkItem*)Context)->OnCallBack();
	}

	VOID OnCallBack()
	{
		m_st = (static_cast<T *>(m_pOwer)->*m_pInvoker)(m_lpParam);
		m_event.Set();
	}

private:
	CKeEvent m_event;
	void* m_pOwer;
	LPVOID m_lpParam;
	P_INVOKER m_pInvoker;
	NTSTATUS m_st;
	PDEVICE_OBJECT m_pDeviceObject;
};


};