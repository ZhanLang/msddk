#pragma once
#include <uframe/frame.h>
class CUFrame
{
public:
	CUFrame(LPCWSTR lpszDrvName,frame_cb cb, void* param);

public:
	BOOL Create();
	VOID Close();

	HRESULT SendMessage(int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
public:
	virtual int OnMsg(int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
private:
	CString m_strDrvName;
	void* m_lpc;
	frame_cb m_cb;
	void* m_param;
};