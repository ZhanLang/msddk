#pragma once
#include "device.h"
namespace msddk { ;

class CKPnpDevice : public CDevice
{
public:
	CKPnpDevice(DEVICE_TYPE DeviceType = FILE_DEVICE_UNKNOWN,
		LPCWSTR pwszDeviceName = NULL,
		ULONG DeviceCharacteristics = FILE_DEVICE_SECURE_OPEN,
		bool bExclusive = FALSE,
		ULONG AdditionalDeviceFlags = DO_POWER_PAGABLE
		):CDevice(DeviceType, pwszDeviceName, DeviceCharacteristics, bExclusive, AdditionalDeviceFlags)
	{

	}

	virtual ~CKPnpDevice()
	{}
};

};