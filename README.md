
[TOC]
## 为什么要造这个轮子
### 1、提高开发效率
 * 充分利用C++的对象管理的优势来管理内核对象和内存的生命周期。
 * 把内核对象抽象为C++对象降低驱动开发的门槛。
 * 提供常用的数据结构，map,vector,string,buffer，像使用STL一样使用简单。
 * 提供多种资源锁，event，mutex等
 * 提供多种线程的使用场景
 * 提供内核与应用层的双向过程调用。
 ### 2、减少出问题的几率

*  把指针限制在对象的生命周期内。
*  限制线程的生命周期，在对象生命周期结束时等待线程结束。

### 3、降低驱动开发门槛

*     因为已经将内核开发中的大多数对象抽象为C++对象，使开发者的精力主要集中在业务的处理上。
*     封装了绝大多数内核API，开发驱动像开发应用一样简单。

## 基础数据结构
### CKeString  

*  位置#include<kutil/string.h>
*  能力：
    *  常用的字符串操作与ATL::Cstring 一致
    *  与UNICODE_STRING或者ANSI_STRING转换。
* 实例：
>```
>CKeStringW sMsg = L"Hello";
>sMsg += L"Ma.Guojun";
>PUNICODE_STRING pStr = sMsg;
>```
---
### CKeMap 
*  位置#include<kutil/map.h>
*  能力：
    *  内核字典
    *  添加、删除、查找、遍历
* 示例：

>**插入**
```
CKeMap<int, CKeString> strMap;
strMap.Insert(1, L"hello");
strMap.Insert(4, L"msddk");
````
>**遍历**
```
CKeMap<int,CKeString>::Iterator it = strMap.Min();
for (; it ; it++)
{
    SKeString sValue = it->Value;
}
```

>**查找**
```
CKeMap<int,CKeString>::ValueType value = strMap.Find(2);
```
>**删除**
```
strMap.Remove(2);
```

---
### CKeVector 
*  位置#include<kutil/vector.h>
*  能力：
    *  内核容器
    *  添加、删除、查找、遍历、排序
* 示例：

>**插入**
```
CKeVector<CKeStringW> sVector;
sVector.Add(L"Ma");
sVector.Insert(0, L"Guo");
```
>**删除**
```
sVector.Delete(0, 1);
```
>**排序**
```
sVector.Sort();
```

>**遍历**
```
for ( int nLoop = 0 ; nLoop < sVector.Size() ; nLoop++)
{
       CKeStringW sValue = sVector[nLoop];
}
```
>**查找**
```
for ( int nLoop = 0 ; nLoop < sVector.Size() ; nLoop++)
{
       CKeStringW sValue = sVector[nLoop];
}
```

## 基础线程对象
### CKeThread 
*  位置#include<kthread/thread.h>
*  能力：
    *  内核线程对象
    * 启动、终止线程
* 示例：

>**声明**
```
class MyKeThread : public CKeThread
{
protected:
       virtual int ThreadBody()
       {
              KdPrint(("MyKeThread is running\n"));
              return 0;
       }
};
```
>**启动线程**
```
MyKeThread keThread;
keThread.Start();
 ```
 >**停止线程**
```
keThread.Join();
 ```
 ---
 ### CKeFunctionThread 
*  位置#include<kthread/function_thread.h>
*  能力：
    *  内核线程对象,使用全局函数作为线程入口。
    * 启动、终止线程
* 示例：

```
int MyFuncThreadRoute(void *lpParam)
{
       UNREFERENCED_PARAMETER(lpParam);
       KdPrint(("MyFuncThread\n"));
       return 0;
}

CKeFunctionThread* pFuncThread = new CKeFunctionThread(&MyFuncThreadRoute, this);
pFuncThread->Start();
pFuncThread->Join();
delete pFuncThread;
```


 ---
 ### CKeMemberThread 
*  位置#include<kthread/member_thread.h>
*  能力：
    *  内核线程对象,使用成员方法作为入口。
    * 启动、终止线程
* 示例：
```
class UseMemberThread
{
public:
       UseMemberThread() :
                    m_MemberThread(this, &UseMemberThread::MyMemberThreadRoute)
       {
            m_MemberThread.Start();
       }
       ~UseMemberThread()
       {
              m_MemberThread.Join();
       }
protected:
       virtual int MyMemberThreadRoute()
       {
            KdPrint(("MyMemberThreadRoute\n"));
            return 0;
       }
private:
    CKeMemberThread m_MemberThread;
};
```
---
## 基础同步对象
### CKeEvent

*  位置#include<ksync/event.h>
*  能力：
    *  内核事件
* 示例：
 >**声明**
 
```
CKeEvent event;
```

 >**设置为有信号状态**
 
```
event.Set();
```

 >**设置为无信号状态**
 
```
event.Reset();
```
 >**等待**
 
```
event.Wait();
```

### CKeMutex
*  位置#include<ksync/mutex.h>
*  能力：
    *  内核互斥体
* 示例：
 >**声明**
 
```
CKeMutex mutex;
```
 >**上锁**
```
mutex.Lock();
```
>**解锁**
```
mutex.Unlock();
```


### CKeFastMutex
*  位置#include<ksync/fastmutex.h>
*  能力：
    *  内核快速互斥体
* 示例：
 >**声明**
 
```
CKeFastMutex mutex;
```
 >**上锁**
```
mutex.Lock();
```
>**解锁**
```
mutex.Unlock();
```


### CKeRWLock
*  位置#include<ksync/rwlock.h>
*  能力：
    *  内核读写锁（资源锁）
* 示例：
 >**声明**
 
```
CKeRWLock rwLock;
```
 >**读上锁**
```
rwLock.LockRead();
```
>**读解锁**
```
rwLock.UnlockRead();
```
 >**写上锁**
```
rwLock.LockWrite();
```
>**写解锁**
```
rwLock.UnlockWrite();
```


### CKeSemaphore
*  位置#include<ksync/semaphore.h>
*  能力：
    *  内核信号量
* 示例：
 >**声明**
 
```
CKeSemaphore semp;
```
 >**设置为有信号状态**
```
semp.Signal();
```
>**等待信号**
```
semp.Wait();
```

### CKeSpinLock
*  位置#include<ksync/spinlock.h>
*  能力：
    *  内核自旋锁
* 示例：
 >**声明**
 
```
CKeSpinLock spinLock;
```
 >**上锁**
```
spinLock.Lock();
```
>**解锁**
```
spinLock.Unlock();
```
### CKeFastLocker
*  位置#include<ksync/autlock.h>
*  能力：
    *  内核自旋锁
* 示例：
 >**声明**
 
```
CKeFastLocker spinLock;
```
 >**上锁**
```
spinLock.Lock();
```
>**解锁**
```
spinLock.Unlock();
```