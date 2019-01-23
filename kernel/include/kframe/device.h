#pragma once
#include<kutil\string.h>
#include "irp.h"

#define BEGIN_DISPATCH_ROUTINE \
virtual NTSTATUS __forceinline DispatchRoutine(IN IncomingIrp *Irp, IO_STACK_LOCATION *IrpSp){\
	NTSTATUS status = STATUS_SUCCESS;\
	switch(IrpSp->MajorFunction){;

#define END_DISPATCH_ROUTINE \
	default:\
			return __super::DispatchRoutine(Irp, IrpSp);\
		}\
		Irp->SetIoStatus(status);\
		Irp->CompleteRequest();\
		return status;}

#define DISPATCH_ROUTINE(Mj, Disp)\
	case Mj:\
	status = Disp(Irp,IrpSp);break;

#define BEGIN_DISPATCH_IO_CONTROL\
	case IRP_MJ_DEVICE_CONTROL:{\
	ULONG uMethod = METHOD_FROM_CTL_CODE(IrpSp->Parameters.DeviceIoControl.IoControlCode);\
	ULONG uCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

#define IO_CONTROL_BUFFERED(code,func)\
	if (uMethod == METHOD_BUFFERED && uCode == code)\
	{\
		status = func(\
			IrpSp->Parameters.DeviceIoControl.IoControlCode,\
			Irp->GetSystemBuffer(),\
			IrpSp->Parameters.DeviceIoControl.InputBufferLength,\
			Irp->GetSystemBuffer(),\
			IrpSp->Parameters.DeviceIoControl.OutputBufferLength);break;\
	}\

#define  END_DISPATCH_IO_CONTROL };break;

namespace msddk { ;
class CDriver;
class CDevice
{
public:
	CDevice(
		DEVICE_TYPE DeviceType,			/*设备类型*/
		const wchar_t *pwszDeviceName,  /*设备名称*/
		ULONG DeviceCharacteristics,	/*设备特征*/
		bool bExclusive,				/*独占设备*/
		ULONG AdditionalDeviceFlags		/*fdo->Flags*/
	);
	virtual ~CDevice();



public:
	bool Valid();
	/*挂载到设备栈*/
	NTSTATUS AttachToDeviceStack(PDEVICE_OBJECT DeviceObject);
	NTSTATUS AttachToDevice(CKeStringW DevicePath);
	NTSTATUS RegisterInterface(IN CONST GUID *pGuid, IN PCUNICODE_STRING ReferenceString = NULL);

	//创建设备
	//内部调用IoCreateDevice
	//pDriver:驱动对象
	//bCompleteInitialization 创建完成后标记初始化
	//pwszLinkPath 设备连接名称
	NTSTATUS CreateDevice(CDriver *pDriver, bool bCompleteInitialization = true, const wchar_t *pwszLinkPath = NULL);
	NTSTATUS DeleteDevice(bool FromIRPHandler);
	NTSTATUS DetachDevice();
	NTSTATUS EnableInterface();
	NTSTATUS DisableInterface();


	//标记设备完成初始化
	//m_pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	void CompleteInitialization();

protected:
	virtual NTSTATUS OnAfterCreate();
	virtual NTSTATUS OnBeforeDelete();

protected:
	virtual LPCWSTR GetDeviceName();
	virtual LPCWSTR GetDeviceLinkName();

public://一下是私有函数，在框架中调用。
	struct Extension
	{
		enum { DefaultSignature = 'VEDB' };
		unsigned Signature;
		CDevice *pDevice;
	};

	NTSTATUS ProcessIRP(IN PIRP  Irp, bool bIsPowerIrp);
	NTSTATUS PostProcessIRP(IncomingIrp *pIrp, NTSTATUS ProcessingStatus, bool FromDispatcherThread);

protected:
	virtual NTSTATUS __forceinline DispatchRoutine(IN IncomingIrp *Irp, IO_STACK_LOCATION *IrpSp);

private:
	NTSTATUS ForwardPacketToNextDriver(IN IncomingIrp *Irp);
	NTSTATUS ForwardPacketToNextDriverWithIrpCompletion(IN IncomingIrp *Irp);

protected:
	static NTSTATUS IrpCompletingCompletionRoutine(IN PDEVICE_OBJECT  DeviceObject, IN PIRP Irp, IN PVOID Context);
private:


private:
	CKeStringW m_DeviceName;
	DEVICE_TYPE m_DeviceType;
	bool m_bExclusive;
	bool m_bDeletePending;
	bool m_bInterfaceEnabled;
	ULONG m_AdditionalDeviceFlags;
	PDEVICE_OBJECT m_pDeviceObject;
	PDEVICE_OBJECT m_pNextDevice;
	PDEVICE_OBJECT m_pUnderlyingPDO;
	UNICODE_STRING m_InterfaceName;
	CKeStringW m_LinkName;
	ULONG m_DeviceCharacteristics;
	CDriver* m_pDriver;
};

};
