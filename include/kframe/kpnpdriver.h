#pragma once

#include"driver.h"
namespace msddk { ;
template<class _DeviceClass, GUID *_pDeviceInterfaceGuid = NULL>
class CKGenericPnpDriver : public CKDriver
{
public:
	_DeviceClass * m_pDevice;
	CKGenericPnpDriver():CKDriver(true)
	{
		m_pDevice = NULL;
	}

	virtual ~CKGenericPnpDriver()
	{
		if (m_pDevice)
		{
			delete m_pDevice;
			m_pDevice = NULL;
		}
	}

	virtual NTSTATUS AddDevice(IN PDEVICE_OBJECT  PhysicalDeviceObject) override
	{
		if (m_pDevice && m_pDevice->Valid())
			return STATUS_ALREADY_REGISTERED;

		m_pDevice = new _DeviceClass();
		if (!m_pDevice)
			return STATUS_NO_MEMORY;

		NTSTATUS st = m_pDevice->AddDevice(this, PhysicalDeviceObject, _pDeviceInterfaceGuid, NULL);
		return st;

	}
};

};
