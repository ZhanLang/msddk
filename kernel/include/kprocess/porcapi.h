#pragma once
#include <ntifs.h>
#include <kutil/string.h>
#include <kutil/types.h>

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
	static NTSTATUS GetProcessImagePath(IN PEPROCESS pEprocess, CKeStringW& ImagePath)
	{
		HANDLE hProcess = NULL;
		NTSTATUS Status = STATUS_SUCCESS;
		ULONG returnedLength;
		ULONG bufferLength;
		PVOID buffer;
		PUNICODE_STRING imageName;

		static QUERY_INFO_PROCESS ZwQueryInformationProcess = NULL;
		PAGED_CODE();

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
			return Status;

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

	static NTSTATUS GetProcessImagePath(IN HANDLE dwProcessId, CKeStringW& ImagePath)
	{
		NTSTATUS Status;
		HANDLE hProcess;
		PEPROCESS pEprocess;
		PAGED_CODE();
		Status = PsLookupProcessByProcessId((HANDLE)dwProcessId, &pEprocess);
		if (!NT_SUCCESS(Status))
			return Status;

		Status = ObOpenObjectByPointer(pEprocess, OBJ_KERNEL_HANDLE, NULL, GENERIC_READ, *PsProcessType, KernelMode, &hProcess);
		if (!NT_SUCCESS(Status))
			return Status;

		return GetProcessImagePath(pEprocess, ImagePath);
	}
};

};
