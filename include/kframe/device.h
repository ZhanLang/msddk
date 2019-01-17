#pragma once
#include<util\kstring.h>
namespace msddk { ;
class CKDriver;
class CKDevice
{
	CKStringW m_DeviceName;
	DEVICE_TYPE m_DeviceType;
	bool m_bExclusive;
	ULONG m_AdditionalDeviceFlags;
	PDEVICE_OBJECT m_pDeviceObject;
	PDEVICE_OBJECT m_pNextDevice;
	PDEVICE_OBJECT m_pUnderlyingPDO;
	UNICODE_STRING m_InterfaceName;
	CKStringW m_LinkName;
	ULONG m_DeviceCharacteristics;
public:
	CKDevice(DEVICE_TYPE DeviceType, const wchar_t *pwszDeviceName, ULONG DeviceCharacteristics, bool bExclusive,ULONG AdditionalDeviceFlags)
	{
		if (pwszDeviceName)
			m_DeviceName = pwszDeviceName;

		m_DeviceCharacteristics = DeviceCharacteristics;
		m_pNextDevice = NULL;
		m_bExclusive = bExclusive;
		m_pDeviceObject = NULL;
		m_DeviceType = DeviceType;
		m_pUnderlyingPDO = NULL;
		m_AdditionalDeviceFlags = AdditionalDeviceFlags;
	}
	~CKDevice()
	{}
public:
	struct Extension
	{
		enum { DefaultSignature = 'VEDB' };
		unsigned Signature;
		CKDevice *pDevice;
	};



public:
	bool Valid()
	{
		return (m_pDeviceObject != NULL);
	}

	

	NTSTATUS AttachToDeviceStack(PDEVICE_OBJECT DeviceObject)
	{
		if (!m_pDeviceObject)
			return STATUS_INVALID_DEVICE_STATE;
		if (!(m_pDeviceObject->Flags & DO_DEVICE_INITIALIZING))
			return STATUS_INVALID_DEVICE_STATE;
		if (m_pNextDevice)
			return STATUS_ALREADY_REGISTERED;
		m_pNextDevice = IoAttachDeviceToDeviceStack(m_pDeviceObject, DeviceObject);
		if (!m_pNextDevice)
			return STATUS_INVALID_DEVICE_STATE;
		m_pUnderlyingPDO = DeviceObject;
		return STATUS_SUCCESS;
	}

	NTSTATUS RegisterInterface(IN CONST GUID *pGuid, IN PCUNICODE_STRING ReferenceString = NULL)
	{
		if (!pGuid)
			return STATUS_INVALID_PARAMETER;
		if (!m_pUnderlyingPDO || m_InterfaceName.Buffer)
			return STATUS_INVALID_DEVICE_STATE;
		return IoRegisterDeviceInterface(m_pUnderlyingPDO, pGuid, (PUNICODE_STRING)ReferenceString, &m_InterfaceName);
	}
	virtual NTSTATUS AddDevice(CKDriver *pDriver, PDEVICE_OBJECT PhysicalDeviceObject, const GUID *pInterfaceGuid, const wchar_t *pwszLinkPath);

	NTSTATUS RegisterDevice(CKDriver *pDriver, bool bCompleteInitialization = true, const wchar_t *pwszLinkPath = NULL);
public://一下是私有函数，在框架中调用。


	NTSTATUS ProcessIRP(IN PIRP  Irp, bool bIsPowerIrp)
	{
		UNREFERENCED_PARAMETER(Irp);
		UNREFERENCED_PARAMETER(bIsPowerIrp);
		return STATUS_SUCCESS;
	}

private:


private:
	
};

};
