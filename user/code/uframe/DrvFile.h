#pragma once
class CDrvFile
{
public:
	CDrvFile(LPCWSTR lpszFile);
	~CDrvFile(void);
	HRESULT DeviceIoControl(DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,LPVOID lpOutBuffer, DWORD nOutBufferSize,LPDWORD lpBytesReturned);
private:
	CString m_strDrvName;
	HANDLE m_hFile;
};

