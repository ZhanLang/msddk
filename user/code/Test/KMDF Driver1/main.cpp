


#include <kprocess/porcapi.h>
#include <wdm.h>
#include <kframe/frame.h>
#include <kframe/simple_drive.h>
#include <kframe/unknown_device.h>
#include <klpc/client.h>

struct PROC_MSG
{
	WCHAR SourceImagePath[MAX_PATH];
	WCHAR TargetImagePath[MAX_PATH];
};

using namespace msddk;
class MyDevicer : public CUnknownDevcie
{
public:
	MyDevicer()
	{
		m_pObHandle = NULL;
	}
	virtual ~MyDevicer()
	{

	}


	static OB_PREOP_CALLBACK_STATUS
		obPreCall(
			PVOID RegistrationContext,
			POB_PRE_OPERATION_INFORMATION pOperationInformation
		)
	{
		UNREFERENCED_PARAMETER(RegistrationContext);
		UNREFERENCED_PARAMETER(pOperationInformation);
		ULONG oper = pOperationInformation->Operation;


		if ( oper == OB_OPERATION_HANDLE_CREATE )
		{
			ULONG oriAcess = pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess;

			if (oriAcess == PROCESS_ALL_ACCESS)
			{
				return OB_PREOP_SUCCESS;
			}

			if (oriAcess & 0x0001 /*PROCESS_TERMINATE*/ || oriAcess & PROCESS_DUP_HANDLE)
			{
				HANDLE hCurProcess = PsGetCurrentProcessId();
				CKeStringW sImagePath;
				CKeProcess::GetProcessImagePath(hCurProcess, sImagePath);
				KdPrint(("curprocess = %ws\n", sImagePath.GetBuffer()));


				PEPROCESS pEprocess = (PEPROCESS)pOperationInformation->Object;
				CKeStringW sTargetImagePath;
				CKeProcess::GetProcessImagePath(pEprocess, sTargetImagePath);
				KdPrint(("sTargetImagePath = %ws\n", sTargetImagePath.GetBuffer()));

				PROC_MSG msg = { 0 };
				wcscpy_s(msg.TargetImagePath, _countof(msg.TargetImagePath), sTargetImagePath.GetBuffer());
				wcscpy_s(msg.SourceImagePath, _countof(msg.SourceImagePath), sImagePath.GetBuffer());

				INT nRet = -1;
				CKeLpcClient(L"\\ProcMsg").SendMessage(0, &msg, sizeof(msg), NULL, 0, NULL, &nRet);

				if ( nRet == 1)
				{
					if (oriAcess & 0x0001 )
					{
						pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~0x0001;
					}
					if (oriAcess & PROCESS_DUP_HANDLE)
					{
						pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_DUP_HANDLE;
					}
				}

				KdPrint(("nRet = %d\n", nRet));
			}
		}
		return OB_PREOP_SUCCESS;
	}


	virtual NTSTATUS OnAfterCreate()
	{
		NTSTATUS st = STATUS_SUCCESS;
		OB_CALLBACK_REGISTRATION obReg = { 0 };
		obReg.Version = ObGetFilterVersion();
		obReg.OperationRegistrationCount = 1;
		obReg.RegistrationContext = NULL;
		RtlInitUnicodeString(&obReg.Altitude, L"321000");

		OB_OPERATION_REGISTRATION opReg = { 0 };
		opReg.ObjectType = PsProcessType;
		opReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
		opReg.PreOperation = &obPreCall;
		obReg.OperationRegistration = &opReg;

		KdPrint(("m_pObHandle=%x\n", m_pObHandle));
		st = ObRegisterCallbacks(&obReg, &m_pObHandle); //在这里注册回调函数
		if ( !NT_SUCCESS(st) )
		{
			KdPrint(("ObRegisterCallbacks failed"));
		}

		KdPrint(("m_pObHandle=%x\n", m_pObHandle));

		return st;
	}

	virtual NTSTATUS OnBeforeDelete()
	{
		KdPrint(("OnBeforeDelete handle=%x\n", m_pObHandle));
		if ( m_pObHandle != NULL)
		{
			ObUnRegisterCallbacks(m_pObHandle);
			m_pObHandle = NULL;
		}
		return STATUS_SUCCESS;
	}


private:
	PVOID m_pObHandle;
};

DRIVER_ENTRY(SimpleDrive<MyDevicer>);