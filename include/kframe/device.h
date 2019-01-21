#pragma once
#include<kutil\string.h>
//#include "irp.h"
namespace msddk { ;
class CDriver;
class CDevice
{
public:
	CDevice(
		DEVICE_TYPE DeviceType,			/*�豸����*/
		const wchar_t *pwszDeviceName,  /*�豸����*/
		ULONG DeviceCharacteristics,	/*�豸����*/
		bool bExclusive,				/*��ռ�豸*/
		ULONG AdditionalDeviceFlags		/*fdo->Flags*/
	);
	virtual ~CDevice();



public:
	bool Valid();
	/*���ص��豸ջ*/
	NTSTATUS AttachToDeviceStack(PDEVICE_OBJECT DeviceObject);
	NTSTATUS AttachToDevice(CKeStringW DevicePath);
	NTSTATUS RegisterInterface(IN CONST GUID *pGuid, IN PCUNICODE_STRING ReferenceString = NULL);

	//�����豸
	//�ڲ�����IoCreateDevice
	//pDriver:��������
	//bCompleteInitialization ������ɺ��ǳ�ʼ��
	//pwszLinkPath �豸��������
	NTSTATUS CreateDevice(CDriver *pDriver, bool bCompleteInitialization = true, const wchar_t *pwszLinkPath = NULL);
	NTSTATUS DeleteDevice(bool FromIRPHandler);
	NTSTATUS DetachDevice();
	NTSTATUS EnableInterface();
	NTSTATUS DisableInterface();


	//����豸��ɳ�ʼ��
	//m_pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	void CompleteInitialization();

protected:
	virtual NTSTATUS OnAfterCreate();
	virtual NTSTATUS OnBeforeDelete();

protected:
	virtual LPCWSTR GetDeviceName();
	virtual LPCWSTR GetDeviceLinkName();

public://һ����˽�к������ڿ���е��á�
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
