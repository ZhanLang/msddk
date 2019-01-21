#pragma once
#include<kutil\string.h>
//#include "irp.h"
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

	NTSTATUS ProcessIRP(IN PIRP  Irp, bool bIsPowerIrp)
	{
		UNREFERENCED_PARAMETER(Irp);
		UNREFERENCED_PARAMETER(bIsPowerIrp);
		return STATUS_SUCCESS;
	}

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
