


#include <kprocess/porcapi.h>
#include <wdm.h>
#include <kframe/frame.h>
#include <kframe/simple_drive.h>
#include <kframe/unknown_device.h>
#include <klpc/client.h>
#include <klog/log.h>
#include <klog/logworker.h>
#include <ktime/time.h>
#include <kthread/member_thread.h>
#include <kthread/workitem.h>
#include <kutil/vector.h>
#include <kutil/map.h>

#include <kutil/memory.hpp>

using namespace msddk;
CKeLogWorker* pLogWorker;
class MyDevicer : public CUnknownDevcie 
{
public:
	MyDevicer()
	{
		m_pObHandle = NULL;
	}
	virtual ~MyDevicer()
	{

	}


	virtual NTSTATUS OnAfterCreate()
	{
		NTSTATUS st = STATUS_SUCCESS;
		KdPrint(("OnAfterCreate"));
		//strVector = new CKeVector<DWORD,VectorNonPagePool>();
		Map = new CKeMap<DWORD, CKeNpStringW, NonPagedObject>();
		//Map->Insert(1, L"1111");
		for ( int n = 0; n < 1; n++)
		{
			//strVector->Add(1111);
		}
		return st;
	}

	virtual NTSTATUS OnBeforeDelete()
	{
		//delete strVector;
		return STATUS_SUCCESS;
	}


private:
	CKeVector<DWORD,VectorNonPagePool> *strVector;
	CKeMap<DWORD, CKeNpStringW, NonPagedObject>* Map;
	PVOID m_pObHandle;
};

DRIVER_ENTRY(SimpleDrive<MyDevicer>);