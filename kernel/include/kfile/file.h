#pragma once
namespace msddk { ;

struct KeFileMode
{
	ACCESS_MASK DesiredAccess;
	ULONG FileAttributes;
	ULONG ShareAccess;
	ULONG CreateDisposition;
	ULONG CreateOptions;
	ULONG ObjectAttributes;

	KeFileMode ShareAll() const
	{
		KeFileMode copy = *this;
		copy.ShareAccess |= FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
		return copy;
	}

	KeFileMode AddAttributes(ULONG FileAttr) const
	{
		KeFileMode copy = *this;
		copy.FileAttributes |= FileAttr;
		return copy;
	}

	KeFileMode AddCreateOptions(ULONG flags) const
	{
		KeFileMode copy = *this;
		copy.CreateOptions |= flags;
		return copy;
	}
};
enum SeekType
{
	FileBegin = 0,
	FileCurrent = 1,
	FileEnd = 2,
};
class CKeFile
{
protected:
	HANDLE m_hFile;

public:
	bool Valid();
	void Close();
	

public:
	NTSTATUS AddFileAttributes(ULONG Attr);
	NTSTATUS RemoveFileAttributes(ULONG Attr);
	NTSTATUS Delete();
	NTSTATUS Move(const CKeStringW &NewPath, bool DeleteIfExists);
	NTSTATUS ReadAt(CKeBuffer& Buf, ULONGLONG offset, size_t size);
	NTSTATUS WriteAt(const CKeBuffer& Buf);
	LONGLONG GetPosition();
	LONGLONG Seek(LONGLONG Offset, SeekType seekType);
public:
	static NTSTATUS Delete(const CKeStringW&, bool ForceDeleteReadonly = false);
	static bool Exists(const CKeStringW& Path);
};

};