// uframe.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "uframe.h"
#include <ulpc/lpc.h>
#include "DrvFile.h"

typedef struct _CLIENT_MESSAGE_INFO_
{
	int			nRet;		/*�����÷�����ֵ*/
	int			nMsgType;	/*��Ϣ����*/
	int			nMsgID;		/*��ϢID*/
	int			nProd;		/*��ƷID*/
	int			nSubProd;	/*�Ӳ�ƷID*/
	int			nCPId;		/*�����߽���ID*/
	int			nCTId;		/*�������߳�ID*/
	int			nMsgSize;	/*��Ϣ���С*/
	int			nRetBufSize;/*�����߷����ڴ�Ĵ�С*/

}CLIENT_MSG, *PCLIENT_MSG;

CUFrame::CUFrame(LPCWSTR lpszDrvName,frame_cb cb, void* param)
{
	m_lpc = NULL;
	m_strDrvName = lpszDrvName;
	m_cb = cb;
	m_param = param;
}

int _stdcall frame_lpc_cb(void* param,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
{
	CUFrame* pFrame = (CUFrame*)param;
	return pFrame->OnMsg(uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize);
}
BOOL CUFrame::Create()
{
	CString strLpcName;
	strLpcName.Format(L"\\%s", m_strDrvName);
	m_lpc = lpc_server_create(strLpcName, this, frame_lpc_cb );
	if ( !m_lpc )
	{
		return FALSE;
	}

	return TRUE;
}

VOID CUFrame::Close()
{
	if ( m_lpc )
	{
		lpc_server_close(m_lpc);
		m_lpc = NULL;
	}

}

HRESULT CUFrame::SendMessage(int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
{
	return CDrvFile(m_strDrvName).DeviceIoControl( uCode, pInBuf, nInCch, pOutBuf, nOutCch,LPDWORD(nOutSize));
}

int CUFrame::OnMsg(int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
{
	if ( m_cb )
		return m_cb(m_param,uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize);

	return ERROR_NOINTERFACE;
}

extern"C"
{
	UFRAME_API void*	__cdecl frame_create(const wchar_t* drvName, void* param, frame_cb cb)
	{
		CUFrame* pFrame = new CUFrame(drvName, cb, param);
		if ( !pFrame->Create() )
		{
			pFrame->Close();
			delete pFrame;
			pFrame = NULL;
		}
		
		return pFrame;
	}

	UFRAME_API void		__cdecl frame_close(void* frame)
	{
		if ( frame )
		{
			CUFrame* pFrame = (CUFrame*)frame;
			pFrame->Close();
			delete pFrame;
		}
	}

	UFRAME_API HRESULT	__cdecl frame_send(void* frame,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
	{
		if ( frame )
		{
			CUFrame* pFrame = (CUFrame*)frame;
			return pFrame->SendMessage(uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize);
		}
		return ERROR_INVALID_PARAMETER;
	}
};
