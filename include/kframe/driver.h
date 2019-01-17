#pragma once
#include<util/kstring.h>
#include"device.h"

namespace msddk { ;

class CKDriver
{
	
protected:
	
	PDRIVER_OBJECT m_DriverObject;
	CKStringW m_RegistryPath;
	bool m_bRegisterAddDevice;
public:
	friend class CKDevice;
	CKDriver(bool RegisterAddDevice = false)
	{
		m_DriverObject = NULL;
		m_bRegisterAddDevice = RegisterAddDevice;


	}
	~CKDriver()
	{

	}


protected:
	virtual NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp, bool bIsPowerIrp)
	{
		if (!DeviceObject || !Irp)
			return STATUS_ACCESS_VIOLATION;
		CKDevice::Extension *pExt = (CKDevice::Extension *)DeviceObject->DeviceExtension;
		if (!pExt || (pExt->Signature != CKDevice::Extension::DefaultSignature) || !pExt->pDevice)
			return STATUS_INVALID_DEVICE_STATE;
		return pExt->pDevice->ProcessIRP(Irp, bIsPowerIrp);
	}

protected:
	virtual void OnDeviceRegistered(const CKDevice *pDevice) 
	{
		UNREFERENCED_PARAMETER(pDevice);
	}
	virtual void OnDeviceUnregistered(const CKDevice *pDevice) 
	{
		UNREFERENCED_PARAMETER(pDevice);
	}

	virtual NTSTATUS AddDevice(IN PDEVICE_OBJECT  PhysicalDeviceObject)
	{
		UNREFERENCED_PARAMETER(PhysicalDeviceObject);
		return STATUS_NOT_SUPPORTED;
	}
	virtual NTSTATUS DriverLoad(IN PUNICODE_STRING RegistryPath)
	{
		UNREFERENCED_PARAMETER(RegistryPath);

		m_DriverObject->DriverUnload = sDriverUnload;

		for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
			m_DriverObject->MajorFunction[i] = sDispatch;

		m_DriverObject->MajorFunction[IRP_MJ_POWER] = sDispatchPower;
		if (m_bRegisterAddDevice)
			m_DriverObject->DriverExtension->AddDevice = sAddDevice;

		return STATUS_SUCCESS;
	}
public:
	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
	{
		KdPrint(("DriverEntry\r"));
		m_DriverObject = DriverObject;
		return DriverLoad(RegistryPath);
	}

	static VOID		sDriverUnload(IN PDRIVER_OBJECT DriverObject)
	{
		UNREFERENCED_PARAMETER(DriverObject);
		if (s_MainDriver)
			delete s_MainDriver;
		s_MainDriver = NULL;
	}

	static NTSTATUS sDispatch(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp)
	{
		if (!s_MainDriver)
			return STATUS_INTERNAL_ERROR;
		return s_MainDriver->DispatchRoutine(DeviceObject, Irp, false);
	}

	static NTSTATUS sDispatchPower(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp)
	{
		if (!s_MainDriver)
			return STATUS_INTERNAL_ERROR;
		return s_MainDriver->DispatchRoutine(DeviceObject, Irp, true);
	}

	static NTSTATUS sAddDevice(IN PDRIVER_OBJECT  DriverObject, IN PDEVICE_OBJECT  PhysicalDeviceObject)
	{
		UNREFERENCED_PARAMETER(DriverObject);
		if (!s_MainDriver)
			return STATUS_INTERNAL_ERROR;
		return s_MainDriver->AddDevice(PhysicalDeviceObject);
	}

	static CKDriver *GetMainDriver()
	{
		return s_MainDriver;
	}
public:
	static CKDriver *s_MainDriver;
};

};

//msddk::CKDriver *_stdcall CreateMainDriverInstance();