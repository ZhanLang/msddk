#pragma once
#include "types.h"
#include"memory.h"


namespace msddk{;

struct StrPageMem : public PagedObject{
	VOID* Malloc(size_t size){
		VOID* p = ExAllocatePoolWithTag(PagedPool, size, 'SPag');
 		if ( p )
 			memset(p, 0, size);
		
		return p;
	}
	VOID Free(VOID* lpVoid){
		if (lpVoid)
			ExFreePoolWithTag(lpVoid, 'SPag');
	}
};

struct StrNonPageMem : public NonPagedObject{
	VOID* Malloc(size_t size){
		VOID* p = ExAllocatePoolWithTag(NonPagedPool, size, 'SNPa');
 		if (p)
 			memset(p, 0, size);
		return p;
	}
	VOID Free(VOID* lpVoid){
		if (lpVoid)
			ExFreePoolWithTag(lpVoid, 'SNPa');
	}
};

template<typename T,typename NTStr, typename M = StrPageMem>
class CKeStringBase : public M
{
public:
	typedef NTStr* PNTStr ;
	enum{ INVALID_POS = -1};  //错误的长度，位置等
	CKeStringBase();
	CKeStringBase(T c);
	CKeStringBase(const T *chars);
	CKeStringBase(const CKeStringBase &s);
	CKeStringBase(const PNTStr &s);
	~CKeStringBase();
	operator const T*() const ;
	T Back() const ;

	T* GetBuffer();
	T* GetBufferSetLength(int minBufLenght);
	void ReleaseBuffer() ;
	void ReleaseBuffer(int newLength);

	CKeStringBase& operator=(T c);
	CKeStringBase& operator=(const T *chars);
	CKeStringBase& operator=(const PNTStr ntStr);
	CKeStringBase& operator=(const CKeStringBase& s);
	CKeStringBase& operator+=(T c);
	CKeStringBase& operator+=(const T *s);
	CKeStringBase& operator+=(const CKeStringBase &s);
	CKeStringBase& operator+=(const PNTStr &s);
	operator const PNTStr();
	CKeStringBase Mid(int startIndex) const;
	CKeStringBase Mid(int startIndex, int count) const;
	CKeStringBase Left(int count) const;
	CKeStringBase Right(int count) const;

	void MakeUpper();
	void MakeLower(); 
	
	int Compare(const CKeStringBase<T,NTStr,M>& s) const;
	int Compare(const T *s) const;
	int CompareNoCase(const CKeStringBase& s) const;
	int CompareNoCase(const T *s) const;

	int Find(T c) const ;
	int Find(T c, int startIndex) const;
	int Find(const CKeStringBase &s) const ;
	int Find(const CKeStringBase &s, int startIndex) const;
	int ReverseFind(T c) const;
	int FindOneOf(const CKeStringBase &s) const;

	void TrimLeft(T c);
	void TrimLeft();
	void TrimRight();
	void TrimRight(T c);
	void Trim();

	int Insert(int index, const CKeStringBase &s);
	int Insert(int index, T c);
	int Append(T c);
	int Append(const CKeStringBase &s);

	int Format(const T* pstrFormat, ...);
	int FormatV(const wchar_t* pszFormat, va_list args );
	int FormatV(const char* pszFormat, va_list args );

	int AppendFormat(const T* pstrFormat, ...);
	void Empty();

	int Length() const;
	bool IsEmpty() const ;

	int Replace(T oldChar, T newChar);
	int Replace(const CKeStringBase &oldString, const CKeStringBase &newString);

	int Delete(int index, int count = 1);
	void DeleteBack();
	void TrimLeftWithCharSet(const CKeStringBase &charSet);
	void TrimRightWithCharSet(const CKeStringBase &charSet);
private:

	inline void GrowLength(int n);
	inline void CorrectIndex(int &index) const;
	CKeStringBase GetTrimDefaultCharSet();
	inline T * StringCopy(T *dest, const T *src);
	inline T * StringNCopy(T *dest, const T *src,int n);
	inline int StringLen(const T *s);
	inline const T* GetNextCharPointer(const T *p)const;
	inline const T* GetPrevCharPointer(const T *, const T *p)const;
	inline void SetCapacity(int newCapacity);
	void MoveItems(int destIndex, int srcIndex);
	void InsertSpace(int &index, int size);
	inline int StringCompare(const char *s1, const char *s2) const;
	inline int StringCompare(const wchar_t *s1, const wchar_t *s2) const;
	inline int StringCompareNoCase(const T *s1, const T *s2) const;
	inline T CharUpper(T c) const ;
	inline T CharLower(T c) const;
private:
	T *_chars;
	int _length;
	int _capacity;
	NTStr _ntStr;
};



template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s, T c);

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(T c, const CKeStringBase<T,NTStr,M>& s);

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s, const T * chars);


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const T * chars, const CKeStringBase<T,NTStr,M>& s);

template<typename T,typename NTStr,typename M>
bool operator==(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
bool operator<(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
bool operator==(const T *s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
bool operator==(const CKeStringBase<T,NTStr,M>& s1, const T *s2);

template<typename T,typename NTStr,typename M>
bool operator!=(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
bool operator!=(const T *s1, const CKeStringBase<T,NTStr,M>& s2);

template<typename T,typename NTStr,typename M>
bool operator!=(const CKeStringBase<T,NTStr,M>& s1, const T *s2);

typedef CKeStringBase<char, ANSI_STRING , StrNonPageMem>   CKeStringA;
typedef CKeStringBase<wchar_t, UNICODE_STRING, StrNonPageMem>CKeStringW;

typedef CKeStringBase<char, ANSI_STRING, StrPageMem>   CKePageStringA;
typedef CKeStringBase<wchar_t, UNICODE_STRING, StrPageMem>CKePageStringW;


/*-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_*/
template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::CKeStringBase() : _chars(0), _length(0) , _capacity(0) 
{ 
		SetCapacity(3); 
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::CKeStringBase(T c):  _chars(0), _length(0), _capacity(0)
{
	memset(&_ntStr, 0, sizeof(_ntStr));
	SetCapacity(1);
	_chars[0] = c;
	_chars[1] = 0;
	_length = 1;
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::CKeStringBase(const T *chars): _chars(0), _length(0), _capacity(0)
{
	memset(&_ntStr, 0, sizeof(_ntStr));
	if (chars)
	{
		int length = StringLen(chars);
		SetCapacity(length);
		StringCopy(_chars, chars);
		_length = length;
	}
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::CKeStringBase(const CKeStringBase &s):  _chars(0), _length(0), _capacity(0)
{
	memset(&_ntStr, 0, sizeof(_ntStr));
	SetCapacity(s._length);
	StringCopy(_chars, s._chars);
	_length = s._length;
}

template<typename T, typename NTStr, typename M /*= StrPageMem*/>
msddk::CKeStringBase<T, NTStr, M>::CKeStringBase(const PNTStr &ntStr)
{
	memset(&_ntStr, 0, sizeof(_ntStr));
	if (ntStr && ntStr->Buffer && ntStr->Length)
	{
		int length = ntStr->Length / sizeof(T);
		SetCapacity(length);
		StringNCopy(_chars, ntStr->Buffer, length);
		_length = length;
	}
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::~CKeStringBase() 
{  
	delete []_chars;
	_chars = NULL;
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>::operator const T*() const 
{ 
	return _chars;
}

template<typename T,typename NTStr,typename M>
T CKeStringBase<T,NTStr,M>::Back() const 
{ 
	return _chars[_length - 1]; 
}

template<typename T,typename NTStr,typename M>
T* CKeStringBase<T,NTStr,M>::GetBuffer()
{
	return _chars;
}

template<typename T,typename NTStr,typename M>
T* CKeStringBase<T,NTStr,M>::GetBufferSetLength(int minBufLenght)
{
	if (minBufLenght >= _capacity)
	{
		SetCapacity(minBufLenght);
	}
		

	return _chars;
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::ReleaseBuffer() 
{ 
	ReleaseBuffer(StringLen(_chars));
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::ReleaseBuffer(int newLength)
{
	_chars[newLength] = 0;
	_length = newLength;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator=(T c)
{
	Empty();
	SetCapacity(1);
	_chars[0] = c;
	_chars[1] = 0;
	_length = 1;
	return *this;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator=(const T *chars)
{
	Empty();
	if ( chars )
	{
		int length = StringLen(chars);
		SetCapacity(length);
		StringCopy(_chars, chars);
		_length = length;
	}
	
	return *this;
}

template<typename T, typename NTStr, typename M>
CKeStringBase<T, NTStr,M>& CKeStringBase<T, NTStr,M>::operator=(const PNTStr ntStr)
{
	Empty();
	if (ntStr && ntStr->Buffer && ntStr->Length)
	{
		int length = ntStr->Length / sizeof(T);
		SetCapacity(length);
		StringNCopy(_chars, ntStr->Buffer, length);
		_length = length;
	}

	return *this;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator=(const CKeStringBase<T,NTStr,M>& s)
{
	if (&s == this)
		return *this;
	Empty();
	SetCapacity(s._length);
	StringCopy(_chars, s._chars);
	_length = s._length;
	return *this;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator+=(T c)
{
	GrowLength(1);
	_chars[_length] = c;
	_chars[++_length] = 0;
	return *this;
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator+=(const T *s)
{
	if ( s )
	{
		int len = StringLen(s);
		GrowLength(len);
		StringCopy(_chars + _length, s);
		_length += len;
	}
	
	return *this;
}

template<typename T, typename NTStr, typename M>
CKeStringBase<T, NTStr,M>::operator const PNTStr()
{
	_ntStr.Length = (USHORT)(_length * sizeof(T));
	_ntStr.MaximumLength = (USHORT)(_capacity * sizeof(T));
	_ntStr.Buffer = _chars;
	return &_ntStr;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M>& CKeStringBase<T,NTStr,M>::operator+=(const CKeStringBase<T,NTStr,M> &s)
{
	if ( s.Length() )
	{
		GrowLength(s._length);
		StringCopy(_chars + _length, s._chars);
		_length += s._length;
	}
	
	return *this;
}

template<typename T, typename NTStr, typename M /*= StrPageMem*/>
CKeStringBase<T, NTStr, M>& msddk::CKeStringBase<T, NTStr, M>::operator+=(const PNTStr &ntStr)
{
	if (ntStr && ntStr->Buffer && ntStr->Length)
	{
		int length = ntStr->Length / sizeof(T);
		GrowLength(length);
		StringNCopy(_chars + _length, ntStr->Buffer, length);
		_length += length;
	}
	return *this;
}



template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> CKeStringBase<T,NTStr,M>::Mid(int startIndex) const
{ 
	return Mid(startIndex, _length - startIndex); 
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> CKeStringBase<T,NTStr,M>::Mid(int startIndex, int count) const
{
	if (startIndex + count > _length)
		count = _length - startIndex;

	if (count < 1) return (T*)(NULL);

	if (startIndex == 0 && startIndex + count == _length)
		return *this;

	CKeStringBase<T,NTStr,M> result;
	result.SetCapacity(count);

	for (int i = 0; i < count; i++)
		result._chars[i] = _chars[startIndex + i];

	result._chars[count] = 0;
	result._length = count;
	return result;
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> CKeStringBase<T,NTStr,M>::Left(int count) const
{ 
	return Mid(0, count); 
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> CKeStringBase<T,NTStr,M>::Right(int count) const
{
	if (count > _length)
		count = _length;
	return Mid(_length - count, count);
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::MakeUpper() 
{ 
	for (int i = 0; i <= _length; i++)
		_chars[i] = CharUpper(_chars[i]);
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::MakeLower() 
{ 
	for (int i = 0; i <= _length; i++)
		_chars[i] = CharLower(_chars[i]);
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Compare(const CKeStringBase& s) const
{ 
	return StringCompare(_chars, s._chars); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Compare(const T *s) const
{ 
	return StringCompare(_chars, s); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::CompareNoCase(const CKeStringBase& s) const
{ 
	return StringCompareNoCase(_chars, s._chars); 
}


template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::CompareNoCase(const T *s) const
{ 
	return StringCompareNoCase(_chars, s); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Find(T c) const 
{ 
	return Find(c, 0); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Find(T c, int startIndex) const
{
	const T *p = _chars + startIndex;
	for (;;)
	{
		if (*p == c)
			return (int)(p - _chars);

		if (*p == 0)
			return -1;

		p = GetNextCharPointer(p);
	}
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Find(const CKeStringBase<T,NTStr,M> &s) const 
{ 
	return Find(s, 0); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Find(const CKeStringBase<T,NTStr,M> &s, int startIndex) const
{
	if (s.IsEmpty())
		return startIndex;
	for (; startIndex < _length; startIndex++)
	{
		int j;
		for (j = 0; j < s._length && startIndex + j < _length; j++)
			if (_chars[startIndex+j] != s._chars[j])
				break;
		if (j == s._length)
			return startIndex;
	}
	return -1;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::ReverseFind(T c) const
{
	if (_length == 0)
		return -1;

	const T *p = _chars + _length - 1;
	for (;;)
	{
		if (*p == c)
			return (int)(p - _chars);
		if (p == _chars)
			return -1;
		p = GetPrevCharPointer(_chars, p);
	}
}


template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::FindOneOf(const CKeStringBase<T,NTStr,M> &s) const
{
	for (int i = 0; i < _length; i++)
		if (s.Find(_chars[i]) >= 0)
			return i;
	return -1;
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimLeft(T c)
{
	const T *p = _chars;
	while (c == *p)
		p = GetNextCharPointer(p);
	Delete(0, p - _chars);
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimLeft()
{
	TrimLeftWithCharSet(GetTrimDefaultCharSet());
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimRight()
{
	TrimRightWithCharSet(GetTrimDefaultCharSet());
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimRight(T c)
{
	const T *p = _chars;
	const T *pLast = NULL;
	while (*p != 0)
	{
		if (*p == c)
		{
			if (pLast == NULL)
				pLast = p;
		}
		else
			pLast = NULL;
		p = GetNextCharPointer(p);
	}

	if (pLast != NULL)
	{
		int i = pLast - _chars;
		Delete(i, _length - i);
	}
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::Trim()
{
	TrimRight();
	TrimLeft();
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Insert(int index, const CKeStringBase<T,NTStr,M> &s)
{
	CorrectIndex(index);
	if (s.IsEmpty())
		return _length;
	int numInsertChars = s.Length();
	InsertSpace(index, numInsertChars);
	for (int i = 0; i < numInsertChars; i++)
		_chars[index + i] = s[i];
	_length += numInsertChars;
	return _length;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Insert(int index, T c)
{
	InsertSpace(index, 1);
	_chars[index] = c;
	_length++;
	return _length;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Append(T c)
{
	return Insert(Length(),c);
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Append(const CKeStringBase<T,NTStr,M> &s)
{
	return Insert(Length(),s);
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Format(const T* pstrFormat, ...)
{
	va_list argList;
	va_start(argList, pstrFormat);
	int iRet = FormatV(pstrFormat,argList); 
	va_end(argList);
	return iRet;
}

#pragma warning(disable:4996)
template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::FormatV(const char* pszFormat, va_list args )
{
	char* szSprintf = NULL;
	int nLen = 0;
	nLen = ::_vsnprintf(NULL, 0, pszFormat, args);
	szSprintf = (char*) Malloc((nLen + 1)*sizeof(char));
	int iRet = ::_vsnprintf(szSprintf, nLen + 1, pszFormat, args);
	Empty();
	Insert(0,szSprintf);
	Free(szSprintf);
	return iRet;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::FormatV(const wchar_t * pszFormat, va_list args )
{
	wchar_t* szSprintf = NULL;
	int nLen = 0;
	nLen = ::_vsnwprintf(NULL, 0, pszFormat, args);
	szSprintf = (wchar_t*) AllocMem()((nLen + 1)*sizeof(wchar_t));
	memset(szSprintf, 0, (nLen + 1) * sizeof(wchar_t));
	int iRet = ::_vsnwprintf(szSprintf, nLen + 1, pszFormat, args);
	Empty();
	Insert(0,szSprintf);
	FreeMem()(szSprintf);
	return iRet;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::AppendFormat(const T* pstrFormat, ...)
{
	CKeStringBase<T,NTStr,M> s;
	va_list argList;
	va_start(argList, pstrFormat);
	int iRet = s.FormatV(pstrFormat,argList);
	Insert(Length(),s);
	va_end(argList);
	return iRet;
}


template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::Empty()
{
	_length = 0;
	if (_chars) _chars[0] = 0;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Length() const 
{ 
	return _length; 
}

template<typename T,typename NTStr,typename M>
bool CKeStringBase<T,NTStr,M>::IsEmpty() const 
{ 
	return (_length == 0); 
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Replace(T oldChar, T newChar)
{
	if (oldChar == newChar)
		return 0;
	int number  = 0;
	int pos  = 0;
	while (pos < Length())
	{
		pos = Find(oldChar, pos);
		if (pos < 0)
			break;
		_chars[pos] = newChar;
		pos++;
		number++;
	}

	return number;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Replace(const CKeStringBase<T,NTStr,M> &oldString, const CKeStringBase<T,NTStr,M> &newString)
{
	if (oldString.IsEmpty())
		return 0;
	if (oldString == newString)
		return 0;
	int oldStringLength = oldString.Length();
	int newStringLength = newString.Length();
	int number  = 0;
	int pos  = 0;
	while (pos < _length)
	{
		pos = Find(oldString, pos);
		if (pos < 0)
			break;
		Delete(pos, oldStringLength);
		Insert(pos, newString);
		pos += newStringLength;
		number++;
	}
	return number;
}

template<typename T,typename NTStr,typename M>
int CKeStringBase<T,NTStr,M>::Delete(int index, int count)
{
	if (index + count > _length)
		count = _length - index;
	if (count > 0)
	{
		MoveItems(index, index + count);
		_length -= count;
	}
	return _length;
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::DeleteBack() 
{ 
	Delete(_length - 1); 
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimLeftWithCharSet(const CKeStringBase &charSet)
{
	const T *p = _chars;
	while (charSet.Find(*p) >= 0 && (*p != 0))
		p = GetNextCharPointer(p);
	Delete(0, (int)(p - _chars));
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::TrimRightWithCharSet(const CKeStringBase &charSet)
{
	const T *p = _chars;
	const T *pLast = NULL;
	while (*p != 0)
	{
		if (charSet.Find(*p) >= 0)
		{
			if (pLast == NULL)
				pLast = p;
		}
		else
			pLast = NULL;
		p = GetNextCharPointer(p);
	}
	if (pLast != NULL)
	{
		int i = (int)(pLast - _chars);
		Delete(i, _length - i);
	}
}


template<typename T,typename NTStr,typename M>
inline void CKeStringBase<T, NTStr,M>::GrowLength(int n)
{
	int freeSize = _capacity - _length - 1;
	if (n <= freeSize)
		return;

	int delta = 0;
	if (_capacity > 64)
		delta = _capacity / 2;
	else if (_capacity > 8)
		delta = 16;
	else
		delta = 4;

	if (freeSize + delta < n)
		delta = n - freeSize;

	SetCapacity(_capacity + delta);
}

template<typename T,typename NTStr,typename M>
inline void CKeStringBase<T, NTStr,M>::CorrectIndex(int &index) const
{
	if(index > _length)
		index = _length;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> CKeStringBase<T,NTStr,M>::GetTrimDefaultCharSet()
{
	CKeStringBase<T,NTStr,M> charSet;
	charSet += (T)' ';
	charSet += (T)'\n';
	charSet += (T)'\t';
	return charSet;
}

template<typename T,typename NTStr,typename M>
inline T * CKeStringBase<T,NTStr,M>::StringCopy(T *dest, const T *src)
{
	if (dest && src)
	{
		T *destStart = dest;
		while ((*dest++ = *src++) != 0);
		return destStart;
	}
	
	return NULL;
}

template<typename T, typename NTStr, typename M>
inline T * CKeStringBase<T, NTStr, M>::StringNCopy(T *dest, const T *src, int n)
{
	if (dest && src && n)
	{
		T *destStart = dest;
		for ( int i=0 ;i < n ; i++)
			dest[i] = src[i];

		destStart[n] = '\0';
		return destStart;
	}

	return NULL;
}


template<typename T,typename NTStr,typename M>
inline int CKeStringBase<T,NTStr,M>::StringLen(const T *s)
{
	if(!s) return 0;

	int i;
	for (i = 0; s[i] != '\0'; i++);
	return i;
}

template<typename T,typename NTStr,typename M>
inline const T* CKeStringBase<T,NTStr,M>::GetNextCharPointer(const T *p)const
{ 
	return (p + 1); 
}

template<typename T,typename NTStr,typename M>
inline const T* CKeStringBase<T,NTStr,M>::GetPrevCharPointer(const T *, const T *p)const
{ 
	return (p - 1); 
}

template<typename T,typename NTStr,typename M>
inline void CKeStringBase<T, NTStr,M>::SetCapacity(int newCapacity)
{
	int realCapacity = newCapacity + 1;
	if (realCapacity <= _capacity)
		return;

	T *newBuffer = (T*)Malloc(realCapacity*sizeof(T));
	for (int i = 0; i < _length; i++)
		newBuffer[i] = _chars[i];
		
	newBuffer[_length] = '\0';
	

	Free(_chars);
	_chars = newBuffer;
	_capacity = realCapacity;
}


template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::MoveItems(int destIndex, int srcIndex)
{
	memmove(_chars + destIndex, _chars + srcIndex,sizeof(T) * (_length - srcIndex + 1));
}

template<typename T,typename NTStr,typename M>
void CKeStringBase<T, NTStr,M>::InsertSpace(int &index, int size)
{
	CorrectIndex(index);
	GrowLength(size);
	MoveItems(index + size, index);
}

template<typename T,typename NTStr,typename M>
inline int CKeStringBase<T,NTStr,M>::StringCompare(const char *s1, const char *s2) const
{
	if (s1 && s2)
	{
		for (;;)
		{
			unsigned char c1 = (unsigned char)*s1++;
			unsigned char c2 = (unsigned char)*s2++;
			if (c1 < c2) return -1;
			if (c1 > c2) return 1;
			if (c1 == 0) return 0;
		}
	}

	return -1;
}

template<typename T,typename NTStr,typename M>
inline int CKeStringBase<T,NTStr,M>::StringCompare(const wchar_t *s1, const wchar_t *s2) const
{	
	if (s1 && s2)
	{
		for (;;)
		{
			wchar_t c1 = *s1++;
			wchar_t c2 = *s2++;
			if (c1 < c2) return -1;
			if (c1 > c2) return 1;
			if (c1 == 0) return 0;
		}
	}
	
	return -1;
}

template<typename T,typename NTStr,typename M>
inline T CKeStringBase<T,NTStr,M>::CharUpper(T c) const 
{
	if (c>='a'&&c<='z'){ 
		c = (c - 32);
	}
	return c;
}

template<typename T,typename NTStr,typename M>
inline T CKeStringBase<T,NTStr,M>::CharLower(T c) const 
{
	if (c>='A'&&c<='Z'){ 
		c = (c + 32);
	}
	return c;
}

template<typename T,typename NTStr,typename M>
inline int CKeStringBase<T,NTStr,M>::StringCompareNoCase(const T *s1, const T *s2) const
{
	if (s1 && s2){
		for (;;){
			T c1 = *s1++;T c2 = *s2++;
			if (c1 != c2){
				T u1 = CharUpper(c1);
				T u2 = CharUpper(c2);
				if (u1 < u2) return -1;
				if (u1 > u2) return 1;
			}
			if (c1 == 0) return 0;
		}
	}

	return -1;
}


template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2)
{
	CKeStringBase<T,NTStr,M> result(s1);
	result += s2;
	return result;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s, T c)
{
	CKeStringBase<T,NTStr,M> result(s);
	result += c;
	return result;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(T c, const CKeStringBase<T,NTStr,M>& s)
{
	CKeStringBase<T,NTStr,M> result(c);
	result += s;
	return result;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const CKeStringBase<T,NTStr,M>& s, const T * chars)
{
	CKeStringBase<T,NTStr,M> result(s);
	result += chars;
	return result;
}

template<typename T,typename NTStr,typename M>
CKeStringBase<T,NTStr,M> operator+(const T * chars, const CKeStringBase<T,NTStr,M>& s)
{
	CKeStringBase<T,NTStr,M> result(chars);
	result += s;
	return result;
}

template<typename T,typename NTStr,typename M>
bool operator==(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2)
{ 
	return (s1.Compare(s2) == 0); 
}

template<typename T,typename NTStr,typename M>
bool operator<(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2)
{ 
	return (s1.Compare(s2) < 0); 
}

template<typename T,typename NTStr,typename M>
bool operator==(const T *s1, const CKeStringBase<T,NTStr,M>& s2)
{ 
	return (s2.Compare(s1) == 0); 
}

template<typename T,typename NTStr,typename M>
bool operator==(const CKeStringBase<T,NTStr,M>& s1, const T *s2)
{
	return (s1.Compare(s2) == 0); 
}

template<typename T,typename NTStr,typename M>
bool operator!=(const CKeStringBase<T,NTStr,M>& s1, const CKeStringBase<T,NTStr,M>& s2)
{ 
	return (s1.Compare(s2) != 0);
}

template<typename T,typename NTStr,typename M>
bool operator!=(const T *s1, const CKeStringBase<T,NTStr,M>& s2)
{ 
	return (s2.Compare(s1) != 0);
}

template<typename T,typename NTStr,typename M>
bool operator!=(const CKeStringBase<T,NTStr,M>& s1, const T *s2)
{ 
	return (s1.Compare(s2) != 0);
}

}//namespace msdk

