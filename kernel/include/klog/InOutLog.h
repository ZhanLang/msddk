#pragma once

#include <kutil/string.h>
namespace msddk { ;

class CKeInOutLog
{
public:
	CKeInOutLog(LPCWSTR Func, LPCWSTR Name)
	{
		wcscpy_s(szName, _countof(szName), Name);
		wcscpy_s(szFunc, _countof(szFunc), Func);
		KdPrint(("+In func:%ws , name:%ws\n", Func, Name));
	}

	~CKeInOutLog()
	{
		KdPrint(("-Out func:%ws , name:%ws\n", szFunc, szName));
	}
	
	WCHAR szName[MAX_PATH];
	WCHAR szFunc[MAX_PATH];

};

};
