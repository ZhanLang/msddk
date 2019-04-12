#pragma once
#include <kutil/string.h>
#include <kutil/dbg.h>
namespace msddk { ;

class CKePath
{
public:
	// \Device\HarddiskVolume1\Windows\explorer.exe
	// To
	// C:\Windows\explorer.exe
	static NTSTATUS NtFileNameToDosFileName(LPCWSTR _NtFileName, CKePageStringW& DosFileName)
	{
		PAGED_CODE();
		NTSTATUS status = STATUS_SUCCESS;
		HANDLE hFile = NULL;
		OBJECT_ATTRIBUTES ObjectAttributes;
		IO_STATUS_BLOCK IoStatusBlock;
		PFILE_OBJECT FileObject = NULL;
		POBJECT_NAME_INFORMATION lpName = NULL;
		CKePageStringW NtFileName = _NtFileName;

		if (NtFileName.Compare(L"System") == 0 ||
			NtFileName.Compare(L"Registry") == 0)
		{
			DosFileName = NtFileName;
			return status;
		}

		do 
		{
			InitializeObjectAttributes(&ObjectAttributes, NtFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenFile(&hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function NtFileNameToDosFileName call ZwOpenFile(%ws) failed. Status is %ws\n", NtFileName.GetBuffer(), MapNTStatus(status)));
				break;
			}
			status = ObReferenceObjectByHandle(hFile, FILE_READ_ATTRIBUTES, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function NtFileNameToDosFileName call ObReferenceObjectByHandle(%ws) failed. Status is %ws\n", NtFileName.GetBuffer(), MapNTStatus(status)));
				break;
			}
			status = IoQueryFileDosDeviceName(FileObject, &lpName);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function NtFileNameToDosFileName call IoQueryFileDosDeviceName(%ws) failed. Status is %ws\n", NtFileName.GetBuffer(), MapNTStatus(status)));
				break;
			}

			DosFileName = &lpName->Name;
		} while (FALSE);
		

		if (lpName)
			ExFreePool(lpName);

		if (FileObject)
			ObDereferenceObject(FileObject);

		if (hFile)
			ZwClose(hFile);
		return status;
	}

	// C:\Windows\explorer.exe
	// To
	// \Device\HarddiskVolume1\Windows\explorer.exe
	static NTSTATUS DosFileNameToNtFileName(LPCWSTR _DosFileName, CKePageStringW& NtFileName)
	{
		PAGED_CODE();
		

		NTSTATUS status = STATUS_UNSUCCESSFUL;
		HANDLE hFile = NULL;
		OBJECT_ATTRIBUTES ObjectAttributes;
		IO_STATUS_BLOCK IoStatusBlock;
		PFILE_OBJECT FileObject = NULL;
		UNICODE_STRING volumeDosName;
		CKePageStringW DosFileName = L"\\??\\";
		DosFileName+=_DosFileName;
		if (DosFileName.Compare(L"System") == 0 ||
			DosFileName.Compare(L"Registry") == 0)
		{
			NtFileName = DosFileName;
			return status;
		}

		do 
		{
			InitializeObjectAttributes(&ObjectAttributes, (PUNICODE_STRING)DosFileName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
			status = ZwOpenFile(&hFile, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function DosFileNameToNtFileName call ZwOpenFile(%ws) failed. Status is %ws\n", _DosFileName, MapNTStatus(status)));
				break;
			}

			status = ObReferenceObjectByHandle(hFile, FILE_READ_ATTRIBUTES, *IoFileObjectType, KernelMode, (PVOID*)&FileObject, NULL);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function DosFileNameToNtFileName call ObReferenceObjectByHandle(%ws) failed. Status is %ws\n", _DosFileName, MapNTStatus(status)));
				
				ZwClose(hFile);
				hFile = NULL;
				break;
			}
			ZwClose(hFile);
			hFile = NULL;

			status = IoVolumeDeviceToDosName(FileObject->DeviceObject, &volumeDosName);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function DosFileNameToNtFileName call IoVolumeDeviceToDosName(%ws) failed. Status is %ws\n", _DosFileName, MapNTStatus(status)));
				break;
			}
			CKePageStringW VolumeDosName = L"\\??\\";
			VolumeDosName += &volumeDosName;
			ExFreePool(volumeDosName.Buffer);
			
			CKePageStringW LinkTarget;
			status = FileMonQuerySymbolicLink(VolumeDosName, LinkTarget);
			if ( !NT_SUCCESS(status) )
			{
				KdPrint(("Function DosFileNameToNtFileName call FileMonQuerySymbolicLink(%ws) failed. Status is %ws\n", _DosFileName, MapNTStatus(status)));
				break;
			}
			
			LinkTarget += &FileObject->FileName;
			NtFileName = LinkTarget;
		} while (FALSE);
		
	
		if (FileObject)
			ObDereferenceObject(FileObject);

		return status;
	}

	static NTSTATUS  FileMonQuerySymbolicLink(LPCWSTR _SymbolicLinkName, CKePageStringW& LinkTarget)
	{
		PAGED_CODE();
		OBJECT_ATTRIBUTES ObjectAttributes;
		HANDLE LinkHandle = NULL;
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		CKePageStringW SymbolicLinkName = _SymbolicLinkName;

		do 
		{
			InitializeObjectAttributes(&ObjectAttributes, SymbolicLinkName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);
			status  = ZwOpenSymbolicLinkObject(&LinkHandle, GENERIC_READ, &ObjectAttributes);
			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function FileMonQuerySymbolicLink call ZwOpenSymbolicLinkObject(%ws) failed. Status is %ws\n", _SymbolicLinkName, MapNTStatus(status)));
				break;
			}

			LinkTarget.GetBufferSetLength(MAX_PATH);
			status = ZwQuerySymbolicLinkObject(LinkHandle, LinkTarget, NULL);
			LinkTarget.ReleaseBuffer();

			if (!NT_SUCCESS(status))
			{
				KdPrint(("Function FileMonQuerySymbolicLink call ZwQuerySymbolicLinkObject(%ws) failed. Status is %ws\n", _SymbolicLinkName, MapNTStatus(status)));
				ASSERT(FALSE);
			}
		} while (FALSE);
		
		if ( LinkHandle )
		{
			ZwClose(LinkHandle);
		}

		return status;
	}
	

	static VOID RemoveLinkTarge(CKeStringW& FileName);
};

};