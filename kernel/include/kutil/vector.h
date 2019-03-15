#pragma once
#include "memory.h"
namespace msddk{;

struct VectorPagePool : public PagedObject{
	VOID* Malloc(size_t size){
		return ExAllocatePoolWithTag(PagedPool, size, 'VPag');
	}
	VOID Free(VOID* lpVoid){
		if (lpVoid)
			ExFreePoolWithTag(lpVoid, 'VPag');
	}
};

struct VectorNonPagePool : public NonPagedObject{
	VOID* Malloc(size_t size){
		return ExAllocatePoolWithTag(NonPagedPool, size, 'VNPa');
	}
	VOID Free(VOID* lpVoid){
		if (lpVoid)
			ExFreePoolWithTag(lpVoid, 'VNPa');
	}
};

template<class T,class M=VectorPagePool>
class CKeVectorBaseImp : public M
{
public:
	CKeVectorBaseImp(size_t itemSize);
	virtual ~CKeVectorBaseImp();
	void ClearAndFree();
	int Size() const ;
	bool IsEmpty() const ;
	bool Reserve(int newCapacity);
	void ReserveDown();
	virtual void Delete(int index, int num = 1);
	void Clear();
	void DeleteFrom(int index);
	void DeleteBack();

	
protected:
	void ReserveOnePosition();
	void InsertOneItem(int index);
	void TestIndexAndCorrectNum(int index, int &num) const;

protected:
	int _capacity;
	int _size;
	void *_items;
	size_t _itemSize;

private:
	void MoveItems(int destIndex, int srcIndex);
};


template<typename T, class M = VectorPagePool>
class  CKeVectorBase: public CKeVectorBaseImp<T,M>
{
public:
	CKeVectorBase();
	CKeVectorBase(const CKeVectorBase &v);
	CKeVectorBase& operator=(const CKeVectorBase &v);
	CKeVectorBase& operator+=(const CKeVectorBase &v);
	int Add(T item);
	void Insert(int index, T item);
	const T& operator[](int index) const ;
	T& operator[](int index) ;
	const T& Front() const ;
	T& Front() ;
	const T& Back() const ;
	T& Back() ;
	void Swap(int i, int j);
	int FindInSorted(const T& item, int left, int right) const;
	int FindInSorted(const T& item) const;
	int AddToUniqueSorted(const T& item);
	static void SortRefDown(T* p, int k, int size, int (*compare)(const T*, const T*, void *), void *param);
	void Sort(int (*compare)(const T*, const T*, void *), void *param);
};

template <class T, class M = VectorPagePool>
class CKeVector: public CKeVectorBase<void*,M>
{
public:
	CKeVector() ;
	~CKeVector() ;
	CKeVector(const CKeVector &v);
	CKeVector& operator=(const CKeVector &v);
	CKeVector& operator+=(const CKeVector &v);
	const T& operator[](int index) const ;
	T& operator[](int index) ;
	T& Front() ;
	const T& Front() const ;
	T& Back() ;
	const T& Back() const ;
	int Add(const T& item) ;
	void Insert(int index, const T& item) ;
	virtual void Delete(int index, int num = 1);
	int Find(const T& item) const;
	int FindInSorted(const T& item) const;
	int AddToSorted(const T& item);
	void Sort(int (*compare)(void *const *, void *const *, void *), void *param);
	static int CompareObjectItems(void *const *a1, void *const *a2, void * /* param */);
	void Sort() ;
private:
	inline int Compare(T a, T b);
};



/*_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-*/
template<typename T,typename M>
CKeVectorBaseImp<T,M>::CKeVectorBaseImp(size_t itemSize)
	: _capacity(0)
	, _size(0)
	, _items(0)
	, _itemSize(itemSize) 
{

}

template<typename T,typename M>
CKeVectorBaseImp<T,M>::~CKeVectorBaseImp()
{
	ClearAndFree();
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::ClearAndFree()
{
	Clear();
	Free((unsigned char *)_items);
	_capacity = 0;
	_size = 0;
	_items = 0;
}

template<typename T,typename M>
int CKeVectorBaseImp<T,M>::Size() const 
{
	return _size;
}

template<typename T,typename M>
bool CKeVectorBaseImp<T,M>::IsEmpty() const 
{ 
	return (_size == 0); 
}

template<typename T,typename M>
bool CKeVectorBaseImp<T,M>::Reserve(int newCapacity)
{
	if (newCapacity == _capacity)
		return true;

	if ((unsigned)newCapacity >= ((unsigned)1 << (sizeof(unsigned) * 8 - 1)))
		return false;

	size_t newSize = (size_t)(unsigned)newCapacity * _itemSize;
	if (newSize / _itemSize != (size_t)(unsigned)newCapacity)
		return false;

	unsigned char *p = NULL;
	if (newSize > 0)
	{
		p = (unsigned char*)Malloc(newSize);
		if (p == 0)
			return false;

		int numRecordsToMove = (_size < newCapacity ? _size : newCapacity);
		memcpy(p, _items, _itemSize * numRecordsToMove);
	}

	Free((unsigned char *)_items);
	_items = p;
	_capacity = newCapacity;
	return true;
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::ReserveDown()
{
	Reserve(_size);
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::Delete(int index, int num /*= 1*/)
{
	TestIndexAndCorrectNum(index, num);
	if (num > 0)
	{
		MoveItems(index, index + num);
		_size -= num;
	}
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::Clear()
{
	DeleteFrom(0);
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::DeleteFrom(int index)
{
	Delete(index, _size - index);
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::DeleteBack()
{
	Delete(_size - 1);
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::ReserveOnePosition()
{
	if (_size != _capacity)
		return;
	unsigned delta = 1;
	if (_capacity >= 64)
		delta = (unsigned)_capacity / 4;
	else if (_capacity >= 8)
		delta = 8;
	Reserve(_capacity + (int)delta);
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::InsertOneItem(int index)
{
	ReserveOnePosition();
	MoveItems(index + 1, index);
	_size++;
}

template<typename T,typename M>
void CKeVectorBaseImp<T,M>::TestIndexAndCorrectNum(int index, int &num) const
{ 
	if (index + num > _size) 
		num = _size - index; 
}


template<typename T,typename M>
void CKeVectorBaseImp<T,M>::MoveItems(int destIndex, int srcIndex)
{
	memmove(((unsigned char *)_items) + destIndex * _itemSize,
		((unsigned char  *)_items) + srcIndex * _itemSize,
		_itemSize * (_size - srcIndex));
}



template<typename T,typename M>
CKeVectorBase<T,M>::CKeVectorBase()
	: CKeVectorBaseImp(sizeof(T))
{

};

template<typename T,typename M>
CKeVectorBase<T,M>::CKeVectorBase(const CKeVectorBase &v)
	: CKeVectorBaseImp(sizeof(T)) 
{ 
	*this = v; 
}

template<typename T,typename M>
CKeVectorBase<T,M>& CKeVectorBase<T,M>::operator=(const CKeVectorBase &v)
{
	Clear();
	return (*this += v);
}

template<typename T,typename M>
CKeVectorBase<T,M>& CKeVectorBase<T,M>::operator+=(const CKeVectorBase &v)
{
	int size = v.Size();
	Reserve(Size() + size);
	for (int i = 0; i < size; i++)
		Add(v[i]);
	return *this;
}

template<typename T,typename M>
int CKeVectorBase<T,M>::Add(T item)
{
	ReserveOnePosition();
	((T *)_items)[_size] = item;
	return _size++;
}

template<typename T,typename M>
void CKeVectorBase<T,M>::Insert(int index, T item)
{
	InsertOneItem(index);
	((T *)_items)[index] = item;
}

template<typename T,typename M>
const T& CKeVectorBase<T,M>::operator[](int index) const 
{
	return ((T *)_items)[index]; 
}

template<typename T,typename M>
T& CKeVectorBase<T,M>::operator[](int index) 
{
	return ((T *)_items)[index]; 
}

template<typename T,typename M>
const T& CKeVectorBase<T,M>::Front() const 
{
	return operator[](0); 
}

template<typename T,typename M>
T& CKeVectorBase<T,M>::Front() 
{
	return operator[](0); 
}

template<typename T,typename M>
const T& CKeVectorBase<T,M>::Back() const 
{
	return operator[](_size - 1); 
}

template<typename T,typename M>
T& CKeVectorBase<T,M>::Back() 
{
	return operator[](_size - 1); 
}

template<typename T,typename M>
void CKeVectorBase<T,M>::Swap(int i, int j)
{
	T temp = operator[](i);
	operator[](i) = operator[](j);
	operator[](j) = temp;
}

template<typename T,typename M>
int CKeVectorBase<T,M>::FindInSorted(const T& item, int left, int right) const
{
	while (left != right)
	{
		int mid = (left + right) / 2;
		const T& midValue = (*this)[mid];
		if (item == midValue)
			return mid;
		if (item < midValue)
			right = mid;
		else
			left = mid + 1;
	}
	return -1;
}

template<typename T,typename M>
int CKeVectorBase<T,M>::FindInSorted(const T& item) const
{
	int left = 0, right = Size();
	while (left != right)
	{
		int mid = (left + right) / 2;
		const T& midValue = (*this)[mid];
		if (item == midValue)
			return mid;
		if (item < midValue)
			right = mid;
		else
			left = mid + 1;
	}
	return -1;
}

template<typename T,typename M>
int CKeVectorBase<T,M>::AddToUniqueSorted(const T& item)
{
	int left = 0, right = Size();
	while (left != right)
	{
		int mid = (left + right) / 2;
		const T& midValue = (*this)[mid];
		if (item == midValue)
			return mid;
		if (item < midValue)
			right = mid;
		else
			left = mid + 1;
	}
	Insert(right, item);
	return right;
}

template<typename T,typename M>
static void CKeVectorBase<T,M>::SortRefDown(T* p, int k, int size, int (*compare)(const T*, const T*, void *), void *param)
{
	T temp = p[k];
	for (;;)
	{
		int s = (k << 1);
		if (s > size)
			break;
		if (s < size && compare(p + s + 1, p + s, param) > 0)
			s++;
		if (compare(&temp, p + s, param) >= 0)
			break;
		p[k] = p[s];
		k = s;
	}
	p[k] = temp;
}

template<typename T,typename M>
void CKeVectorBase<T,M>::Sort(int (*compare)(const T*, const T*, void *), void *param)
{
	int size = _size;
	if (size <= 1)
		return;
	T* p = (&Front()) - 1;
	{
		int i = size / 2;
		do
		SortRefDown(p, i, size, compare, param);
		while (--i != 0);
	}
	do
	{
		T temp = p[size];
		p[size--] = p[1];
		p[1] = temp;
		SortRefDown(p, 1, size, compare, param);
	}
	while (size > 1);
}


template<typename T,typename M>
CKeVector<T,M>::CKeVector() 
{

}


template<typename T,typename M>
CKeVector<T,M>::~CKeVector() 
{ 
	Clear(); 
};


template<typename T,typename M>
CKeVector<T,M>::CKeVector(const CKeVector &v)
	: CKeVectorBase<void*,M>() 
{ 
	*this = v; 
}


template<typename T,typename M> 
CKeVector<T,M>& CKeVector<T,M>::operator=(const CKeVector &v)
{
	Clear();
	return (*this += v);
}

template<typename T,typename M> 
CKeVector<T,M>& CKeVector<T,M>::operator+=(const CKeVector &v)
{
	int size = v.Size();
	Reserve(Size() + size);
	for (int i = 0; i < size; i++)
		Add(v[i]);
	return *this;
}

template<typename T,typename M> 
const T& CKeVector<T,M>::operator[](int index) const 
{
	return *((T *)CKeVectorBase<void*,M>::operator[](index)); 
}

template<typename T,typename M> 
T& CKeVector<T,M>::operator[](int index) 
{
	return *((T *)CKeVectorBase<void*,M>::operator[](index)); 
}

template<typename T,typename M> 
T& CKeVector<T,M>::Front() 
{
	return operator[](0); 
}

template<typename T,typename M> 
const T& CKeVector<T,M>::Front() const 
{
	return operator[](0); 
}

template<typename T,typename M> 
T& CKeVector<T,M>::Back() 
{
	return operator[](_size - 1); 
}

template<typename T,typename M> 
const T& CKeVector<T,M>::Back() const 
{
	return operator[](_size - 1); 
}

template<typename T,typename M> 
int CKeVector<T,M>::Add(const T& item) 
{
	return CKeVectorBase<void*,M>::Add(new T(item)); 
}

template<typename T,typename M> 
void CKeVector<T,M>::Insert(int index, const T& item) 
{
	CKeVectorBase<void*,M>::Insert(index, new T(item)); 
}

template<typename T,typename M> 
void CKeVector<T,M>::Delete(int index, int num /*= 1*/)
{
	TestIndexAndCorrectNum(index, num);
	for (int i = 0; i < num; i++)
		delete (T *)(((void **)_items)[index + i]);
	CKeVectorBase<void*,M>::Delete(index, num);
}

template<typename T,typename M> 
int CKeVector<T,M>::Find(const T& item) const
{
	for (int i = 0; i < Size(); i++)
		if (item == (*this)[i])
			return i;
	return -1;
}

template<typename T,typename M> 
int CKeVector<T,M>::FindInSorted(const T& item) const
{
	int left = 0, right = Size();
	while (left != right)
	{
		int mid = (left + right) / 2;
		const T& midValue = (*this)[mid];
		if (item == midValue)
			return mid;
		if (item < midValue)
			right = mid;
		else
			left = mid + 1;
	}
	return -1;
}

template<typename T,typename M> 
int CKeVector<T,M>::AddToSorted(const T& item)
{
	int left = 0, right = Size();
	while (left != right)
	{
		int mid = (left + right) / 2;
		const T& midValue = (*this)[mid];
		if (item == midValue)
		{
			right = mid + 1;
			break;
		}
		if (item < midValue)
			right = mid;
		else
			left = mid + 1;
	}
	Insert(right, item);
	return right;
}

template<typename T,typename M> 
void CKeVector<T,M>::Sort(int (*compare)(void *const *, void *const *, void *), void *param)
{
	CKeVectorBase<void*,M>::Sort(compare, param); 
}

template<typename T,typename M> 
static int CKeVector<T,M>::CompareObjectItems(void *const *a1, void *const *a2, void * /* param */)
{
	return Compare(*(*((const T **)a1)), *(*((const T **)a2))); 
}

template<typename T,typename M> 
void CKeVector<T,M>::Sort() 
{
	CKeVectorBase<void*,M>::Sort(CompareObjectItems, 0); 
}


template<typename T,typename M> 
inline int CKeVector<T,M>::Compare(T a, T b)
{
	return a < b ? -1 : (a == b ? 0 : 1); 
}


};//namespace msddk