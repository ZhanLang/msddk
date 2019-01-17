#pragma once
#include"device.h"
namespace msddk { ;
	NTSTATUS CKDevice::RegisterDevice(CKDriver *pDriver, bool bCompleteInitialization/* = true*/, const wchar_t *pwszLinkPath/* = NULL*/)
	{
		UNREFERENCED_PARAMETER(pDriver);
		UNREFERENCED_PARAMETER(bCompleteInitialization);
		UNREFERENCED_PARAMETER(pwszLinkPath);

		NTSTATUS st = STATUS_SUCCESS;
		CKStringW FullDevicePath;
		if (m_DeviceName)
		{
			FullDevicePath = L"\\Device\\";
			FullDevicePath += m_DeviceName;
		}

		UNICODE_STRING Str = ToNtStr(FullDevicePath);
		st = IoCreateDevice(pDriver->m_DriverObject,
			sizeof(Extension),
			&Str,
			m_DeviceType,
			m_DeviceCharacteristics,
			m_bExclusive,
			&m_pDeviceObject);

		if (!NT_SUCCESS(st))
		{
			KdPrint(("Device::RegisterDevice(): failed call to IoCreateDevice() (%d)\n", st));
			return st;
		}

		m_pDeviceObject->Flags |= m_AdditionalDeviceFlags;
		Extension *pExt = (Extension *)m_pDeviceObject->DeviceExtension;
		pExt->Signature = Extension::DefaultSignature;
		pExt->pDevice = this;
		if (pwszLinkPath)
		{
			m_LinkName = pwszLinkPath;
			UNICODE_STRING StrLink = ToNtStr(m_LinkName);
			UNICODE_STRING 
			NTSTATUS st = IoCreateSymbolicLink(&StrLink,
				FullDevicePath.ToNTString());
		}
		return STATUS_SUCCESS;
	}

	NTSTATUS CKDevice::AddDevice(CKDriver *pDriver, PDEVICE_OBJECT PhysicalDeviceObject, const GUID *pInterfaceGuid, const wchar_t *pwszLinkPath)
	{
		UNREFERENCED_PARAMETER(pDriver);
		UNREFERENCED_PARAMETER(PhysicalDeviceObject);
		UNREFERENCED_PARAMETER(pInterfaceGuid);
		UNREFERENCED_PARAMETER(pwszLinkPath);

		NTSTATUS st = STATUS_SUCCESS;
		if (PhysicalDeviceObject)
		{
			st = AttachToDeviceStack(PhysicalDeviceObject);
			if (!NT_SUCCESS(st))
				return st;
		}
		if (pInterfaceGuid)
		{
			st = RegisterInterface(pInterfaceGuid);
			if (!NT_SUCCESS(st))
				return st;
		}

		/*
		st = IoCreateDevice(pDriver->m_DriverObject,
			sizeof(Extension),
			FullDevicePath.ToNTString(),
			m_DeviceType,
			m_DeviceCharacteristics,
			m_bExclusive,
			&m_pDeviceObject);
			*/
		return STATUS_SUCCESS;
	}
};
