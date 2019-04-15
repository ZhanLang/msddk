#include "StdAfx.h"
#include "DrvFile.h"


CDrvFile::CDrvFile(LPCWSTR lpszFile)
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_strDrvName.Format(L"\\\\.\\%s", lpszFile);
	m_hFile = CreateFile( m_strDrvName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
}


CDrvFile::~CDrvFile(void)
{
	if ( m_hFile )
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

HRESULT CDrvFile::DeviceIoControl(DWORD dwIoControlCode,LPVOID lpInBuffer,DWORD nInBufferSize,LPVOID lpOutBuffer, DWORD nOutBufferSize,LPDWORD lpBytesReturned)
{
	if ( m_hFile == INVALID_HANDLE_VALUE)
		return E_HANDLE;

	BOOL bRet = ::DeviceIoControl(m_hFile, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, NULL);
	if ( bRet )
		return S_OK;

	return HRESULT_FROM_WIN32(GetLastError());
}
