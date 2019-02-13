#pragma once
#include "device.h"
namespace msddk { ;

class CUnknownDevcie : public CDevice {
public:
	CUnknownDevcie():CDevice(FILE_DEVICE_UNKNOWN,NULL, FILE_DEVICE_SECURE_OPEN,false, DO_POWER_PAGABLE)
	{

	}
	virtual ~CUnknownDevcie()
	{

	}
};

};