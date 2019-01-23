#pragma once
#include "device.h"
#include <defs.h>
namespace msddk { ;

class CDeviceEx : public CDevice
{
public:
	BEGIN_DISPATCH_ROUTINE
		BEGIN_DISPATCH_IO_CONTROL
			IO_CONTROL_BUFFERED(IOCTRL_MSG_REQUEST, OnDeviceControl_1)
		END_DISPATCH_IO_CONTROL
	END_DISPATCH_ROUTINE;

public:
	CDeviceEx(DEVICE_TYPE DeviceType,	/*设备类型*/
		const wchar_t *pwszDeviceName,  /*设备名称*/
		ULONG DeviceCharacteristics,	/*设备特征*/
		bool bExclusive,				/*独占设备*/
		ULONG AdditionalDeviceFlags		/*fdo->Flags*/) :CDevice(DeviceType, pwszDeviceName, DeviceCharacteristics, bExclusive, AdditionalDeviceFlags)
	{
		
	}
	virtual ~CDeviceEx() {
		
	}

	NTSTATUS AskUser(ULONG uCode, void *pInBuffer, ULONG InputLength, void * OutputBuffer, ULONG OutputLength);

protected:
	virtual NTSTATUS OnUserCall(ULONG uCode, void *pInBuffer, ULONG InputLength, void * OutputBuffer, ULONG OutputLength);

private:
	NTSTATUS OnIoCtrlMsgRequest();
	//NTSTATUS OnIoCtrlMsgReply

protected:
	
	virtual NTSTATUS OnAfterCreate()
	{
		return STATUS_SUCCESS;
	}

	virtual NTSTATUS OnBeforeDelete()
	{
		void *pContext = 0;
		while (PIRP pIrp = m_IrpQuery.DequeuePacket(&pContext))
		{

		}
		return STATUS_SUCCESS;
	}

	NTSTATUS OnIrpCreate(IncomingIrp *Irp, IO_STACK_LOCATION *IrpSp)
	{
		UNREFERENCED_PARAMETER(Irp);
		UNREFERENCED_PARAMETER(IrpSp);
		return STATUS_SUCCESS;
	}
	NTSTATUS OnDeviceControl_1(ULONG ControlCode,void *pInBuffer, ULONG InputLength, void * OutputBuffer , ULONG OutputLength)
	{ 
		UNREFERENCED_PARAMETER(ControlCode);
		UNREFERENCED_PARAMETER(pInBuffer);
		UNREFERENCED_PARAMETER(InputLength);
		UNREFERENCED_PARAMETER(OutputBuffer);
		UNREFERENCED_PARAMETER(OutputLength);
		return STATUS_INVALID_DEVICE_REQUEST; 
	}


private:
	CKeIrpQueue m_IrpQuery;
	CKeSpinLock m_UserSpinLock;
	CKeSpinLock m_KernelSprinLock;

	CKeVector<OutgoingIRP*> m_KernelIrp;
};

};