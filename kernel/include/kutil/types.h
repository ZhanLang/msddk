#pragma once

#include<stdarg.h>  //for va_start, etc.
#include<stdio.h>	// for _vsnprintf, _vsnwprintf, getc, getwc
typedef unsigned int        UINT;
typedef			 int        BOOL;
typedef unsigned long       DWORD;
typedef unsigned long       DWORD;
typedef unsigned char		BYTE;
typedef			 BYTE*		LPBYTE;

#define KE_FAILED(x) { NTSTATUS _h_r_ = (x); if(!NT_SUCCESS(_h_r_)) {return _h_r_;} }
namespace msddk { ;



};
