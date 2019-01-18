#pragma once
#include"driver.h"
namespace msddk { ;

template<class DEV>
class SimpleDrive : public CDriver
{
public:
	DEV * m_Device;
	SimpleDrive():CDriver(false)
	{
		m_Device = NULL;
	}
	~SimpleDrive()
	{
		if (m_Device)
		{
			KdPrint(("~SimpleDrive"));
			delete m_Device;
			m_Device = NULL;
		}
	}

protected:
	virtual NTSTATUS DriverLoad(PUNICODE_STRING RegistryPath)
	{
		NTSTATUS st =  __super::DriverLoad(RegistryPath);
		if (!NT_SUCCESS(st))
		{
			KdPrint(("Device::DriverLoad(): failed call to __super::DriverLoad() (%d)\n", st));
			return st;
		}
		m_Device = new DEV;
		if (!m_Device)
			return STATUS_NO_MEMORY;


		st = m_Device->CreateDevice(this);
		if (!NT_SUCCESS(st))
			return st;

		return STATUS_SUCCESS;
	}

};

};