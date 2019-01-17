#pragma once

#include"driver.h"
#include"device.hpp"

#define DRIVER_ENTRY(cls)\
CKDriver* CKDriver::s_MainDriver = NULL;\
extern "C" NTSTATUS _stdcall DriverEntry(\
	IN OUT PDRIVER_OBJECT   DriverObject,\
	IN PUNICODE_STRING      RegistryPath\
)\
{\
	CKDriver::s_MainDriver = new cls;\
	return CKDriver::s_MainDriver->DriverEntry(DriverObject, RegistryPath);\
}

namespace msddk { ;


template<class T>
class CKFrame
{
public:
	CKDriver* CreateMainDriverInstance()
	{
		return new T();
	}


};

};