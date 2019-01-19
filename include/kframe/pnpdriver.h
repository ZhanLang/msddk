#pragma once

#include"driver.h"
namespace msddk { ;
template<class _DeviceClass, GUID *_pDeviceInterfaceGuid = NULL>
class CKPnpDriver : public CDriver
{
public:
	_DeviceClass * m_pDevice;
	CKPnpDriver():CDriver(true)
	{
		m_pDevice = NULL;
	}

	virtual ~CKPnpDriver()
	{
		if (m_pDevice)
		{
			delete m_pDevice;
			m_pDevice = NULL;
		}
	}

	virtual NTSTATUS AddDevice(IN PDEVICE_OBJECT  PhysicalDeviceObject)
	{
		UNREFERENCED_PARAMETER(PhysicalDeviceObject);
		KdPrint(("CKPnpDriver::AddDevice"));
		if (m_pDevice && m_pDevice->Valid())
			return STATUS_ALREADY_REGISTERED;

		m_pDevice = new _DeviceClass();
		if (!m_pDevice)
			return STATUS_NO_MEMORY;

		//NTSTATUS st = m_pDevice->AddDevice(this, PhysicalDeviceObject, _pDeviceInterfaceGuid, NULL);
		return STATUS_SUCCESS;

	}
};

};
