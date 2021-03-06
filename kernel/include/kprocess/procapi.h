#pragma once
#include <ntifs.h>
#include <kutil/string.h>
#include <kutil/types.h>
#include <kfile/path.h>

typedef NTSTATUS(*QUERY_INFO_PROCESS) (
	__in HANDLE ProcessHandle,
	__in PROCESSINFOCLASS ProcessInformationClass,
	__out_bcount(ProcessInformationLength) PVOID ProcessInformation,
	__in ULONG ProcessInformationLength,
	__out_opt PULONG ReturnLength
	);

namespace msddk {;

class CKeProcess {
public:
	static NTSTATUS GetProcessImagePath(PEPROCESS pEprocess, CKePageStringW& ImagePath)
	{
		PAGED_CODE();
		HANDLE hProcess = NULL;
		NTSTATUS Status = STATUS_SUCCESS;
		ULONG returnedLength;
		ULONG bufferLength;
		PVOID buffer;
		PUNICODE_STRING imageName;

		static QUERY_INFO_PROCESS ZwQueryInformationProcess = NULL;

		if (NULL == ZwQueryInformationProcess)
		{
			UNICODE_STRING routineName;
			RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");
			ZwQueryInformationProcess =
				(QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);
			if (NULL == ZwQueryInformationProcess) {
				DbgPrint("Cannot resolve ZwQueryInformationProcess/n");
			}
		}

		Status = ObOpenObjectByPointer(pEprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_READ, *PsProcessType, KernelMode, &hProcess);
		if (!NT_SUCCESS(Status))
		{
			KdPrint(("Function GetProcessImagePath call ObOpenObjectByPointer(%p) failed. Status is %ws\n", pEprocess, MapNTStatus(Status)));
			return Status;
		}
			

		Status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &returnedLength);

		if (STATUS_INFO_LENGTH_MISMATCH != Status) 
		{
			return Status;
		}

		bufferLength = returnedLength - sizeof(UNICODE_STRING);
		buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, 'ipgD');

		if (NULL == buffer) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		Status = ZwQueryInformationProcess(hProcess,
			ProcessImageFileName,
			buffer,
			returnedLength,
			&returnedLength);
		if (NT_SUCCESS(Status)) {
			imageName = (PUNICODE_STRING)buffer;
			ImagePath = imageName;
		}
		ZwClose(hProcess);
		ExFreePool(buffer);

		return Status;
	}

	static NTSTATUS GetProcessImagePath(IN HANDLE dwProcessId, CKePageStringW& ImagePath)
	{
		NTSTATUS Status;
		PEPROCESS pEprocess = NULL;
		PAGED_CODE();
		Status = PsLookupProcessByProcessId((HANDLE)dwProcessId, &pEprocess);
		if (!NT_SUCCESS(Status))
		{
			KdPrint(("Function GetProcessImagePath2 call PsLookupProcessByProcessId(%p) failed. Status is %ws\n", dwProcessId, MapNTStatus(Status)));
			return Status;
		}
			

		return GetProcessImagePath(pEprocess, ImagePath);
	}

	static NTSTATUS GetProcessDosFileName(HANDLE dwProcessId, CKePageStringW& ImagePath)
	{
		NTSTATUS status = GetProcessImagePath(dwProcessId, ImagePath);
		if ( NT_SUCCESS(status ))
		{
			return CKePath::NtFileNameToDosFileName(ImagePath, ImagePath);
		}
		return status;
	}

	static NTSTATUS GetProcessDosFileName(PEPROCESS pEprocess, CKePageStringW& ImagePath)
	{
		NTSTATUS status = GetProcessImagePath(pEprocess, ImagePath);
		if (NT_SUCCESS(status))
		{
			return CKePath::NtFileNameToDosFileName(ImagePath, ImagePath);
		}
		return status;
	}
};

};
