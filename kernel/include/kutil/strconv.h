#pragma once
#include "string.h"
namespace msddk { ; 

template<class StringT = CKeStringA>
StringT u2a(const wchar_t* Str)
{
	UNICODE_STRING UString;
	ANSI_STRING AString;
	StringT String;

	RtlInitUnicodeString(&UString, Str);
	RtlUnicodeStringToAnsiString(&AString, &UString, TRUE);
	String = &AString;
	RtlFreeAnsiString(&AString);
	return String;
}

template<class StringT = CKeStringW>
StringT a2u(const char* Str)
{
	UNICODE_STRING UString;
	ANSI_STRING AString;
	StringT String;

	RtlInitAnsiString(&AString, Str);
	RtlAnsiStringToUnicodeString(&UString, &AString, TRUE);
	String = &UString;
	RtlFreeUnicodeString(&UString);
	return String;
}

template<class StringT = CKeStringA>
StringT u2utf8(const wchar_t* Str)
{
	StringT String;
	ULONG size = 0;
	ULONG strSize = ((ULONG)wcslen(Str) + 1) * sizeof(wchar_t);
	RtlUnicodeToUTF8N(NULL, 0, &size, Str, strSize);
	String.GetBufferSetLength(size+1);
	RtlUnicodeToUTF8N(String.GetBuffer(), size, &size, Str, strSize);
	String.ReleaseBuffer();
	return String;
}

template<class StringT = CKeStringW>
StringT utf82u(const char* Str)
{
	StringT String;
	ULONG size = 0;
	ULONG strSize = ((ULONG)strlen(Str) + 1) * sizeof(char);
	RtlUTF8ToUnicodeN(NULL, 0, &size, Str, strSize);
	String.GetBufferSetLength(size / sizeof(wchar_t) + 1);
	RtlUTF8ToUnicodeN(String.GetBuffer(), size, &size, Str, strSize);
	String.ReleaseBuffer();
	return String;
}


template<class StringT = CKeStringA>
StringT u2multi(const wchar_t* Str)
{
	StringT String;
	ULONG size = 0;
	ULONG strSize = ((ULONG)wcslen(Str) + 1) * sizeof(wchar_t);
	RtlUnicodeToMultiByteN(NULL, 0, &size, Str, strSize);
	String.GetBufferSetLength(size + 1);
	RtlUnicodeToMultiByteN(String.GetBuffer(), size, &size, Str, strSize);
	String.ReleaseBuffer();
	return String;
}

template<class StringT = CKeStringW>
StringT multi2u(const char* Str)
{
	StringT String;
	ULONG size = 0;
	ULONG strSize = ((ULONG)strlen(Str) + 1) * sizeof(char);
	RtlMultiByteToUnicodeN(NULL, 0, &size, Str, strSize);
	String.GetBufferSetLength(size / sizeof(wchar_t) + 1);
	RtlMultiByteToUnicodeN(String.GetBuffer(), size, &size, Str, strSize);
	String.ReleaseBuffer();
	return String;
}

};