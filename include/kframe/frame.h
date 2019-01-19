#pragma once

#include"driver.h"
#include"device.hpp"
#include"driver.hpp"

#define DRIVER_ENTRY(cls)\
CDriver* CDriver::s_MainDriver = NULL;\
extern "C" NTSTATUS _stdcall DriverEntry(\
	IN OUT PDRIVER_OBJECT   DriverObject,\
	IN PUNICODE_STRING      RegistryPath\
)\
{\
	CDriver::s_MainDriver = new cls;\
	return CDriver::s_MainDriver->DriverEntry(DriverObject, RegistryPath);\
}

namespace msddk { ;


template<class T>
class CKFrame
{
public:
	CDriver* CreateMainDriverInstance()
	{
		return new T();
	}


};

};