#pragma once
#include <kutil/string.h>
namespace msddk { ;

class CKePath
{
public:
	// \Device\HarddiskVolume1\Windows\explorer.exe
	// To
	// C:\Windows\explorer.exe
	static NTSTATUS NtFileNameToDosFileName(LPCWSTR NtFileName, CKeStringW& DosFileName)
	{
		NTSTATUS status = STATUS_SUCCESS;
		OBJECT_ATTRIBUTES ObjectAttributes;
		UNICODE_STRING uNtFileName;
		HANDLE hFile = NULL;
		IO_STATUS_BLOCK IoStatusBlock;
		PFILE_OBJECT FileObject = NULL;
		POBJECT_NAME_INFORMATION lpName = NULL;

		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		if (KeGetCurrentIrql() != PASSIVE_LEVEL)
			return STATUS_ACCESS_DENIED;
		
		

		RtlInitUnicodeString(&uNtFileName, NtFileName);
		InitializeObjectAttributes(&ObjectAttributes, &uNtFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

		status = ZwOpenFile(&hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
		if (!NT_SUCCESS(status))
		{
			return status;
		}

		status = ObReferenceObjectByHandle(hFile, FILE_READ_ATTRIBUTES, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL);
		if (!NT_SUCCESS(status))
		{
			ZwClose(hFile);
			return status;
		}

		status = IoQueryFileDosDeviceName(FileObject, &lpName);
		if (!NT_SUCCESS(status))
		{
			ObDereferenceObject(FileObject);
			ZwClose(hFile);
			return status;
		}


		DosFileName = &lpName->Name;
		ObDereferenceObject(FileObject);
		ZwClose(hFile);
		return status;
	}

	// \??\C:\Windows\Windows\explorer.exe
	// To
	// C:\Windows\Windows\explorer.exe
	static VOID RemoveLinkTarge(CKeStringW& FileName);
};

};