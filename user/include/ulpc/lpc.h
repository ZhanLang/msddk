#pragma once

typedef int (_stdcall *lpc_cb)(void* param,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);

extern"C"
{
	void* _stdcall lpc_server_create(const wchar_t* name, void* param, lpc_cb cb);
	void  _stdcall lpc_server_close( void* pSvr);
	int   _stdcall lpc_send(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);
};

class CLpcModuleHelp
{
public:
	void* lpc_server_create(const wchar_t* name, void* param, lpc_cb cb)
	{
		if ( m_p_lpc_server_create )
			return m_p_lpc_server_create(name, param, cb);
		return NULL;
	}
	void  lpc_server_close( void* pSvr)
	{
		if ( m_p_lpc_server_close )
			m_p_lpc_server_close(pSvr);
	}
	int   lpc_send(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
	{
		if ( m_p_lpc_send )
			return m_p_lpc_send(name, uCode, pInBuf, nInCch, pOutBuf, nOutCch, nOutSize);
		
		//STATUS_NOINTERFACE
		return 0xC00002B9L;
	}

public:
	CLpcModuleHelp()
	{
		m_hModule = NULL;
		m_p_lpc_server_create = NULL;
		m_p_lpc_server_close = NULL;
		m_p_lpc_send = NULL;
		LoadLibrary();
	}
	~CLpcModuleHelp()
	{
		FreeLibrary();
	}
private:
	BOOL LoadLibrary()
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		GetCurrentPath(szModulePath, MAX_PATH);

#ifdef _WIN64 
			_tcscat_s( szModulePath	,_countof(szModulePath),_T("\\ulpc_64.dll"));
#else
		if ( IsWindow64() )
			_tcscat_s( szModulePath	,_countof(szModulePath),_T("\\ulpc_64_32.dll"));
		else
			_tcscat_s(	szModulePath, _countof(szModulePath),_T("\\ulpc_32.dll")	);
#endif // _WIN64
		
		m_hModule = ::LoadLibrary( szModulePath );
		if ( !m_hModule )
			return FALSE;

		m_p_lpc_server_create	= (p_lpc_server_create)GetProcAddress(m_hModule,"lpc_server_create");
		m_p_lpc_server_close	= (p_lpc_server_close)GetProcAddress(m_hModule,"lpc_server_close");
		m_p_lpc_send			= (p_lpc_send)GetProcAddress( m_hModule,"lpc_send");

		return TRUE;
	}

	BOOL FreeLibrary()
	{
		m_p_lpc_server_create	= NULL;
		m_p_lpc_server_close	= NULL;
		m_p_lpc_send			= NULL;

		if (m_hModule)
		{
			::FreeLibrary( m_hModule );
			m_hModule = NULL;
		}

		return TRUE;
	}

	BOOL IsWindow64() 
	{ 
		typedef void 
			(__stdcall * PFNGetNativeSystemInfo)
			(LPSYSTEM_INFO lpSystemInfo); 

		PFNGetNativeSystemInfo	pGNSI; 
		SYSTEM_INFO				si = {0}; 

		pGNSI = (PFNGetNativeSystemInfo) 
			GetProcAddress(  GetModuleHandle(L"kernel32.dll"), 
			"GetNativeSystemInfo"			 ); 
		if( NULL != pGNSI )
		{
			pGNSI(&si); 
		}
		else
		{
			GetSystemInfo(&si);
		}

		if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || 
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ) 
		{
			return TRUE; 
		}

		return FALSE; 
	}

	LPCTSTR GetCurrentPath( LPTSTR path , DWORD dwLen)
	{
		ZeroMemory( path, sizeof(TCHAR)*dwLen );
		GetModuleFileName( NULL, path, dwLen );
		TCHAR* pTail = _tcsrchr(path, _T('\\'));
		if ( NULL != pTail )
		{
			*pTail = 0;
		}
		return path;
	}

private:
	HMODULE m_hModule;
	typedef void* (_stdcall *p_lpc_server_create)(const wchar_t* name, void* param, lpc_cb cb);
	typedef void  (_stdcall *p_lpc_server_close)( void* pSvr);
	typedef int   (_stdcall *p_lpc_send)(const wchar_t* name,int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize);

	p_lpc_server_create m_p_lpc_server_create;
	p_lpc_server_close m_p_lpc_server_close;
	p_lpc_send m_p_lpc_send;
};