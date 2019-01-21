#pragma once

#include<ksync/event.h>

extern "C" 
{
NTKERNELAPI PEPROCESS IoGetRequestorProcess( __in PIRP Irp);
NTKERNELAPI BOOLEAN IoIs32bitProcess(IN PIRP  Irp  OPTIONAL);
}

namespace msddk { ;



class IncomingIrp
{
private:
	IRP *m_pIrp;
	long m_Flags;
			
	friend class CDevice;

	IncomingIrp(IRP *pIrp, bool bIsPowerIrp, bool bStartNextPowerIrp)
	{
		ASSERT(pIrp);
		m_pIrp = pIrp;
		m_Flags = bIsPowerIrp ? IsPowerIrp : 0;
		if (bStartNextPowerIrp)
			m_Flags |= StartNextPowerIrp;
	}

	IncomingIrp(IRP *pIrp, long Flags)
	{
		ASSERT(pIrp);
		ASSERT(!(Flags & ~ValidFlags));
		m_pIrp = pIrp;
		m_Flags = Flags;
	}

	~IncomingIrp() {}

public:

	enum
	{
		LowerDriverCalled	= 0x01,
		IrpCompleted		= 0x02,
		IrpMarkedPending	= 0x04,
		IoStatusSet			= 0x08,

		IsPowerIrp			= 0x10,
		StartNextPowerIrp	= 0x20,
		FromDispatchThread	= 0x40,

		ValidFlags			= 0x7F,
	};

	bool IsFromQueue() {return (m_Flags & FromDispatchThread) != 0;}

#pragma region GetXXX methods
	PIO_STATUS_BLOCK GetIoStatus() {return &m_pIrp->IoStatus;}
	void *GetSystemBuffer() {return m_pIrp->AssociatedIrp.SystemBuffer;}
	PMDL GetMdlAddress() {return m_pIrp->MdlAddress;}
	void *GetUserBuffer() {return m_pIrp->UserBuffer;}

	PEPROCESS GetRequestorProcess() {return IoGetRequestorProcess(m_pIrp);}

	NTSTATUS GetStatus() {return m_pIrp->IoStatus.Status;}

	bool Is32BitProcess() {return IoIs32bitProcess(m_pIrp) != FALSE;}
#pragma endregion
			
#pragma region SetXXX methods
	void SetIoStatus(NTSTATUS Status, unsigned Information)
	{
		ASSERT(Status != STATUS_PENDING);
		m_pIrp->IoStatus.Status = Status;
		m_pIrp->IoStatus.Information = Information;
		InterlockedOr(&m_Flags, IoStatusSet);
	}

	void SetIoStatus(NTSTATUS Status)
	{
		ASSERT(Status != STATUS_PENDING);
		m_pIrp->IoStatus.Status = Status;
		InterlockedOr(&m_Flags, IoStatusSet);
	}
#pragma endregion

#pragma region Action methods

	void CompleteRequest(CCHAR PriorityBoost = IO_NO_INCREMENT)
	{
		UNREFERENCED_PARAMETER(PriorityBoost);
		ASSERT(!(m_Flags & IrpCompleted));
		InterlockedOr(&m_Flags, IrpCompleted);
		if (m_Flags & StartNextPowerIrp)
			PoStartNextPowerIrp(m_pIrp);
		IoCompleteRequest(m_pIrp, IO_NO_INCREMENT);
	}

	void MarkPending()
	{
		InterlockedOr(&m_Flags, IrpMarkedPending);
		IoMarkIrpPending(m_pIrp);
	}

#pragma endregion

	//Do not export the IRP pointer directly. Add inline methods instead.
//			IRP *GetIRPPointer() {return m_pIRP;}
};

class OutgoingIRP
{
private:
	PIRP m_pIrp;
	bool m_bSynchronous;
	CKeEvent m_Event;
	IO_STATUS_BLOCK m_StatusBlock;
	PIO_STACK_LOCATION m_pSp;
	PDEVICE_OBJECT m_pDevObj;

public:
	OutgoingIRP(ULONG MajorFunction, UCHAR MinorFunction, PDEVICE_OBJECT pDeviceObject,
		bool Synchronous = true, PVOID pBuffer = NULL, ULONG Length = 0, PLARGE_INTEGER StartingOffset = NULL) :
		m_bSynchronous(Synchronous),
		m_pSp(NULL),
		m_pDevObj(pDeviceObject),
		m_Event(false, kAutoResetEvent)
	{
		ASSERT(Synchronous);
		m_pIrp = IoBuildSynchronousFsdRequest(MajorFunction, pDeviceObject, pBuffer, Length, StartingOffset, m_Event, &m_StatusBlock);
		if (!m_pIrp)
			return;
		m_pSp = IoGetNextIrpStackLocation(m_pIrp);
		m_pSp->MinorFunction = MinorFunction;

		m_pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
		m_pIrp->IoStatus.Information = 0;
	}

	OutgoingIRP(ULONG IoControlCode, PDEVICE_OBJECT pDeviceObject,
		bool InternalControl, PVOID pInBuffer = NULL, ULONG InBufLength = 0, PVOID pOutBuffer = NULL, ULONG OutBufLength = 0) :
		m_bSynchronous(true),
		m_pSp(NULL),
		m_pDevObj(pDeviceObject),
		m_Event(false, kAutoResetEvent)
	{
		m_pIrp = IoBuildDeviceIoControlRequest(IoControlCode, pDeviceObject, pInBuffer, InBufLength, pOutBuffer, OutBufLength, InternalControl, m_Event, &m_StatusBlock);
		if (!m_pIrp)
			return;
		m_pSp = IoGetNextIrpStackLocation(m_pIrp);
	}
	bool Valid() {return (m_pIrp != NULL);}

	NTSTATUS Call(bool Wait = true)
	{
		ASSERT(m_pDevObj);
		ASSERT(m_pIrp);
		NTSTATUS st = IoCallDriver(m_pDevObj, m_pIrp);
		if (!NT_SUCCESS(st))
			return st;
		if (Wait)
			st = m_Event.WaitEx();
		if (!NT_SUCCESS(st))
			return st;
		m_pIrp = NULL;
		return m_StatusBlock.Status;
	}

	NTSTATUS Wait()
	{
		return m_Event.WaitEx();
	}

	PIO_STATUS_BLOCK GetIoStatus() {return &m_StatusBlock;}
			
	~OutgoingIRP()
	{
		//No IoFreeIrp() is needed.
	}

	PIO_STACK_LOCATION GetNextStackLocation()
	{
		return m_pSp;
	}

};
	
};