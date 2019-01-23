#pragma once
#include"device.h"

namespace msddk { ;

CDriver::CDriver(bool RegisterAddDevice /*= false*/)
{
	m_DriverObject = NULL;
	m_bRegisterAddDevice = RegisterAddDevice;
	
}
	
NTSTATUS CDriver::DispatchRoutine(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp, bool bIsPowerIrp)
{
	KdPrint(("DispatchRoutine"));
	if (!DeviceObject || !Irp)
		return STATUS_ACCESS_VIOLATION;
	CDevice::Extension *pExt = (CDevice::Extension *)DeviceObject->DeviceExtension;
	if (!pExt || (pExt->Signature != CDevice::Extension::DefaultSignature) || !pExt->pDevice)
		return STATUS_INVALID_DEVICE_STATE;
	return pExt->pDevice->ProcessIRP(Irp, bIsPowerIrp);
}

void CDriver::OnDeviceRegistered(const CDevice *pDevice)
{
	UNREFERENCED_PARAMETER(pDevice);
}
void CDriver::OnDeviceUnregistered(const CDevice *pDevice)
{
	UNREFERENCED_PARAMETER(pDevice);
}

NTSTATUS CDriver::AddDevice(IN PDEVICE_OBJECT  PhysicalDeviceObject)
{
	UNREFERENCED_PARAMETER(PhysicalDeviceObject);
	KdPrint(("CDriver::AddDevice\n"));
	return STATUS_NOT_SUPPORTED;
}



NTSTATUS CDriver::DriverLoad(PUNICODE_STRING RegistryPath)
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

NTSTATUS CDriver::DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	m_DriverObject = DriverObject;
	m_RegistryPath = RegistryPath;
	
	NTSTATUS st = DriverLoad(RegistryPath);
	if ( !NT_SUCCESS( st ) )
	{
		KdPrint(("CDriver::DriverEntry(): failed call to DriverLoad() (%wS)\n", MapNTStatus(st)));
		return st;
	}

	st = OnAfterInit();
	if ( !NT_SUCCESS( st ) )
	{
		KdPrint(("CDriver::DriverEntry(): failed call to OnAfterInit() (%wS)\n", MapNTStatus(st)));
	}
	return st;
}

NTSTATUS CDriver::OnAfterInit()
{
	KdPrint(("CDriver::OnAfterInit()\n"));
	return STATUS_SUCCESS;
}

NTSTATUS CDriver::OnBeforeUnint()
{
	KdPrint(("CDriver::OnBeforeUnint()\n"));
	return STATUS_SUCCESS;
}

VOID CDriver::sDriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	if ( s_MainDriver )
	{
		NTSTATUS st = s_MainDriver->OnBeforeUnint();
		if ( !NT_SUCCESS( st ) )
		{
			KdPrint(("Device::sDriverUnload(): failed call to OnBeforeUnint() (%wS)\n", MapNTStatus(st)));
		}

		delete s_MainDriver;
		s_MainDriver = NULL;
	}
}

NTSTATUS CDriver::sDispatch(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp)
{
	if (!s_MainDriver)
		return STATUS_INTERNAL_ERROR;
	return s_MainDriver->DispatchRoutine(DeviceObject, Irp, false);
}

NTSTATUS CDriver::sDispatchPower(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp)
{
	if (!s_MainDriver)
		return STATUS_INTERNAL_ERROR;
	return s_MainDriver->DispatchRoutine(DeviceObject, Irp, true);
}

NTSTATUS CDriver::sAddDevice(IN PDRIVER_OBJECT  DriverObject, IN PDEVICE_OBJECT  PhysicalDeviceObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("sAddDevice"));
	if (!s_MainDriver)
		return STATUS_INTERNAL_ERROR;
	return s_MainDriver->AddDevice(PhysicalDeviceObject);
}

CDriver*CDriver::GetMainDriver()
{
	return s_MainDriver;
}

};

//msddk::CDriver *_stdcall CreateMainDriverInstance();