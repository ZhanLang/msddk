//Fishjam Template Library

#pragma once

#ifndef FTL_FAKE_H
#define FTL_FAKE_H

#define FTL_BASE_H

#if defined ATLTRACE
# define FTLTRACE           ATLTRACE
# define FTLASSERT          ATLASSERT

#elif defined TRACE
# define FTLTRACE           TRACE
# define FTLASSERT          ASSERT
#else
#  error must have ATLTRACE or (MFC)TRACE
#endif

#include <functional>

namespace FTL
{

	//禁止拷贝构造和赋值操作符
#define DISABLE_COPY_AND_ASSIGNMENT(className)  \
private:\
	className(const className& ref);\
	className& operator = (const className& ref)

    #ifndef _countof
    #  define _countof(arr) (sizeof(arr) / sizeof(arr[0]))
    #endif

    #define API_VERIFY(x)   \
        bRet = (x); \
        FTLASSERT(TRUE == bRet);

    #define COM_VERIFY(x)   \
        hr = (x); \
        FTLASSERT(SUCCEEDED(hr));

    #define DX_VERIFY(x)   \
        hr = (x); \
        FTLASSERT(SUCCEEDED(hr));

    #ifndef SAFE_FREE_BSTR
    #  define SAFE_FREE_BSTR(s) if(NULL != (s) ){ ::SysFreeString((s)); (s) = NULL; }
    #endif

    #ifndef SAFE_RELEASE
    #  define SAFE_RELEASE(p)  if( NULL != (p) ){ (p)->Release(); (p) = NULL; }
    #endif 

	#ifndef SAFE_DELETE
	#  define SAFE_DELETE(p)	if(NULL != (p) ) { delete p; p = NULL; }
	#endif

    #define CHECK_POINTER_RETURN_VALUE_IF_FAIL(p,r)    \
        if(NULL == p)\
        {\
            FTLASSERT(NULL != p);\
            return r;\
        }

#define FTL_MIN(a,b)                (((a) < (b)) ? (a) : (b))
#define FTL_MAX(a,b)                (((a) > (b)) ? (a) : (b))

#define FTL_CLAMP(x, a, b)			(FTL_MIN((b), FTL_MAX((a), (x))))

#define COMPARE_MEM_LESS(f, o) \
	if( f < o.f ) { return true; }\
		else if( f > o.f ) { return false; }

#  define FTLEXPORT
#  define FTLINLINE inline

#  define DEFAULT_BLOCK_TRACE_THRESHOLD  (100)
#define FUNCTION_BLOCK_TRACE(t)			void(t)

	template <typename T>
	struct UnreferenceLess : public std::binary_function<T, T, bool>
	{
		bool operator()(const T& _Left, const T& _Right) const
		{
			return (*_Left < *_Right);
		}
	};

	template<typename T = CFLockObject>
	class CFAutoLock
	{
	public:
		explicit CFAutoLock<T>(T* pLockObj)
		{
			FTLASSERT(pLockObj);
			m_pLockObj = pLockObj;
			m_pLockObj->Lock(INFINITE);
		}
		~CFAutoLock()
		{
			m_pLockObj->UnLock();
		}
	private:
		T*   m_pLockObj;
	};

#    define SAFE_CLOSE_HANDLE(h,v) if((v) != (h)) { ::CloseHandle((h)); (h) = (v); bRet = bRet; }
#  define SAFE_DELETE_ARRAY(p) if( NULL != (p) ){ delete [] (p); (p) = NULL; }

}
#endif //FTL_FAKE_H