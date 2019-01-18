#pragma once
#include<util/kstring.h>
#include"device.h"

namespace msddk { ;

class CDriver
{
protected:
	PDRIVER_OBJECT m_DriverObject;
	CKStringW m_RegistryPath;
	bool m_bRegisterAddDevice;

public:
	friend class CDevice;
	CDriver(bool RegisterAddDevice = false);
	virtual ~CDriver() {}


protected:
	virtual NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp, bool bIsPowerIrp);

protected:
	//设备注册时通知
	virtual void OnDeviceRegistered(const CDevice *pDevice);
	virtual void OnDeviceUnregistered(const CDevice *pDevice);

	//当有新设备插入时调用
	virtual NTSTATUS AddDevice(IN PDEVICE_OBJECT  PhysicalDeviceObject);

	//初始化驱动设备
	virtual NTSTATUS DriverLoad(PUNICODE_STRING RegistryPath);

public://一下代码由框架调用
	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
	static VOID		sDriverUnload(IN PDRIVER_OBJECT DriverObject);
	static NTSTATUS sDispatch(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp);
	static NTSTATUS sDispatchPower(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp);
	static NTSTATUS sAddDevice(IN PDRIVER_OBJECT  DriverObject, IN PDEVICE_OBJECT  PhysicalDeviceObject);
	static CDriver *GetMainDriver();
public:
	static CDriver *s_MainDriver;
};
};
