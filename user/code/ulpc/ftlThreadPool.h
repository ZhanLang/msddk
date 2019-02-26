///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlThreadPool.h
/// @brief  Fishjam Template Library ThreadPool Header File.
/// @author fujie
/// @version 1.0
/// @date 03/30/2008  -- first version
/// @defgroup ftlThreadPool ftl thread pool function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_THREADPOOL_H
#define FTL_THREADPOOL_H
#pragma once

#include <map>

#ifndef FTL_BASE_H
#  error ftlThreadPool.h requires ftlbase.h to be included first
#endif

#include "ftlThread.h"
//#include "ftlSharePtr.h"
//#include "ftlFunctional.h"

namespace FTL
{
    //! @brief 具有模板参数的线程池类，除能可以方便的进行参数传递外，还拥有以下特点：
    //!  1.能自动根据任务和线程的多少在 最小/最大 线程个数之间调整(Vista后的系统有 SetThreadpoolThreadMaximum 等函数有类似功能)
    //!  2.能方便的对任一任务进行取消，如任务尚未运行则由框架代码处理，如任务已经运行，则需要 JobBase 的子类根据 GetJobWaitType 的返回值进行处理
    //!    (相比较而言，WaitForThreadpoolWorkCallbacks 函数只能取消尚未运行的任务)
    //!  3.能对整个线程池进行 暂停、继续、停止 处理 -- 需要 JobBase 的子类根据 GetJobWaitType 的返回值进行处理
    //!  4.支持回调方式的反馈通知( Progress/Error 等)
    //!  5.使用模版方式实现，能方便的进行参数传递
    //!  6.在加入任务时可以设置优先级
    //!  7.使用的是微软的基本API，能支持WinXP、Vista、Win7等各种操作系统(CreateThreadpoolWork 等只能在Vista后才能使用)
    //!  8.可以有多个实例，方便进行控制(QueueUserWorkItem 等是单实例)
    //!  9.采用的是对称模式的线程池(没有管理线程)，运行成本低
    template <typename T> class CFThreadPool;  

    //enum FJobStatus
    //{
    //	jsWaiting,
    //	jsDoing,
    //	jsCancel,	//不会主动设置为cancel, 如果Job子类支持Cancel的话，自行在 GetJobWaitType 调用后处理并设置
    //	jsDone,
    //	jsError,	
    //};

    //! @brief Job的父类，子类为了实现详细的功能，必须重载 Run 和 OnFinalize 函数,并实例化后放入Pool中等待运行
    //! @todo 如果想实现多个具有不同参数类型的Job，将模板设为 DWORD_PTR 等可以转换为指针的类型即可
    FTLEXPORT template <typename T>
    class CFJobBase
    {
        friend class CFThreadPool<T>;   //允许Threadpool设置 m_pThreadPool/m_nJobIndex 等值
    public:
        //! @param bSuspendOnCreate [in] 设置是否创建挂起的Job，可以用于保证初始化结束(即调用 CFThreadPool::ResumeJob)前Job不会运行
        FTLINLINE CFJobBase(IN BOOL bSuspendOnCreate = FALSE);
        FTLINLINE CFJobBase(IN const T& rJobParam);
        FTLINLINE virtual ~CFJobBase();

        //bool operator < (const CFJobBase & other) const;

        //! @brief 返回Job的优先级，数字越小，优先级越高(在等待队列中拍在越前面)
        FTLINLINE LONG GetPriority() const { return m_nJobPriority; }

        //! @brief 设置Job的优先级, 数字越小，优先级越高(在等待队列中拍在越前面)，缺省值是 0
        //! @warning 该函数必须在 CFThreadPool::SubmitJob 之前调用，
        //!          如果需要调整已经放入Pool的Job优先级，必须通过 CFThreadPool::SetJobPriority 调用
        FTLINLINE LONG SetPriority(LONG nNewPriority);

        //! @brief 返回Job的唯一标识，该值会由Pool设置,Job不可更改
        FTLINLINE LONG GetJobIndex() const;

        //! @brief 返回设置的错误信息 -- 这些错误信息是由子类设置的
        FTLINLINE DWORD GetErrorStatus() const;
        FTLINLINE LPCTSTR GetErrorInfo() const;

        //FTLINLINE FJobStatus GetJobStatus() const { return m_JobStatus; }

        //! @brief Job会使用的参数，此处为了简化，直接采用公有变量的方式
        T		m_JobParam;
    protected:
        //! @brief   子类必须重载的函数，进行实际的业务逻辑
        //! @warning 在这个Run中通常需要循环调用 GetJobWaitType 方法检测Job状态，这样才能支持 暂停、继续、停止 等操作
        virtual BOOL Run() = 0;

        //! @brief 由Pool调用进行资源释放的纯虚函数。如果Job是new出来的，通常需要在该函数中调用 delete this(除非又有另外的生存期管理容器)
        //! @param [in] isWaiting 表示是否是在等待状态下结束的,
        //!   TRUE -- 未运行的Job(直接取消或线程池停止);
        //!   FALSE -- 运行过的Job,可能是运行结束，也可能是运行过程中被取消
        virtual VOID OnFinalize(IN BOOL isWaiting) = 0;
    protected:
        FTLINLINE void _SetErrorStatus(DWORD dwErrorStatus, LPCTSTR pszErrorInfo);
        FTLINLINE void _NotifyProgress(LONGLONG nCurPos, LONGLONG nTotalSize);
        FTLINLINE void _NotifyError();
        FTLINLINE void _NotifyError(DWORD dwError, LPCTSTR pszDescription);
        FTLINLINE void _NotifyCancel();

        //! @brief 通过该函数，获取线程池的状态(Stop/Pause)，以及Job自己的Stop, 用法同 CFThread:GetThreadWaitType,
        //! @param [in] dwMilliseconds 超时值，如果想支持暂停，参数是 INFINITE；如不想支持暂停(如网络传输)，则参数传 0
        FTLINLINE FTLThreadWaitType GetJobWaitType(DWORD dwMilliseconds = INFINITE) const;
    private:
        //设置为私有的变量和方法，即使是子类也不要直接更改，由Pool调用进行控制
        BOOL		m_bSuspendOnCreate;
        LONG		m_nJobPriority;
        LONG		m_nJobIndex;						//! Job的唯一标识，只能由Pool设置
        DWORD				m_dwErrorStatus;
        //CFStringFormater	m_strFormatErrorInfo;		
        CString     m_strFormatErrorInfo;
        HANDLE		m_hEventJobContinue;				//! Job运行时需要满足的事件,如果该事件为非激活状态，则可以使Job暂停
        HANDLE		m_hEventJobStop;					//! 停止Job的事件，该变量将由Pool创建和释放(TODO:Pool中缓存?)
        //FJobStatus	m_JobStatus;
        CFThreadPool<T>* m_pThreadPool;

        //由Pool调用的 暂停、继续、取消等函数 -- Job不能直接调用(Job放入Pool后随时可能结束，用户端保留的指针可能会成为野指针)
        FTLINLINE BOOL _Pause();
        FTLINLINE BOOL _Resume();
        FTLINLINE BOOL _Cancel();
    };


    //! 等待Job的比较函数，用于比较Job的大小来确定在 Waiting 容器中的顺序， 排序依据为 Priority -> Index，
    struct WaitingJobSorter
    {
    public:
        FTLINLINE WaitingJobSorter(LONG nJobPriority, LONG nJobIndex)
        {
            this->nJobPriority = nJobPriority;
            this->nJobIndex = nJobIndex;
        }
        FTLINLINE WaitingJobSorter(const WaitingJobSorter&  rOther)
        {
            nJobPriority = rOther.nJobPriority;
            nJobIndex = rOther.nJobIndex;
        }
        FTLINLINE bool operator < (const WaitingJobSorter & other) const
        {
            COMPARE_MEM_LESS(nJobPriority, other);
            COMPARE_MEM_LESS(nJobIndex, other);
            return false;
        }
    private:
        LONG nJobPriority;
        LONG nJobIndex;
    };

    //! @brief 回调接口，应用逻辑类可以继承该接口并重载相关方法来获取Pool运行的各种状态信息，
    //! 通过 pJob->m_JobParam 可以访问类型为 T 的 参数
    FTLEXPORT template <typename T>
    class IFThreadPoolCallBack
    {
    public:
        //! 当Job运行起来以后，会由 Pool 激发 Begin 和 End 两个函数
        FTLINLINE virtual void OnJobBegin(LONG nJobIndex, CFJobBase<T>* pJob )
        {
            UNREFERENCED_PARAMETER(nJobIndex);
            UNREFERENCED_PARAMETER(pJob);
            //FTLTRACEEX(FTL::tlInfo, TEXT("IFThreadPoolCallBack[0x%x]::OnJobBegin[%d]\n"), this, nJobIndex);
            ATLTRACE(TEXT("IFThreadPoolCallBack[0x%x]::OnJobBegin[%d]\n"), this, nJobIndex);
        } 
        //! 当Job运行起来以后，会由 Pool 激发 Begin 和 End 两个函数
        FTLINLINE virtual void OnJobEnd(LONG nJobIndex, CFJobBase<T>* pJob)
        {
            UNREFERENCED_PARAMETER(nJobIndex);
            UNREFERENCED_PARAMETER(pJob);
            //FTLTRACEEX(FTL::tlInfo, TEXT("IFThreadPoolCallBack[0x%x]::OnJobEnd[%d]\n"), this, nJobIndex);
            ATLTRACE(TEXT("IFThreadPoolCallBack[0x%x]::OnJobEnd[%d]\n"), this, nJobIndex);
        }

        //! 如果尚未到达运行状态就被取消的Job，会由Pool调用这个函数
        FTLINLINE virtual void OnJobCancel(LONG nJobIndex, CFJobBase<T>* pJob)
        {
            UNREFERENCED_PARAMETER(nJobIndex);
            UNREFERENCED_PARAMETER(pJob);
            //FTLTRACEEX(FTL::tlInfo, TEXT("IFThreadPoolCallBack[0x%x]::OnJobCancel[%d]\n"), this, nJobIndex);
            ATLTRACE(TEXT("IFThreadPoolCallBack[0x%x]::OnJobCancel[%d]\n"), this, nJobIndex);
        }

        //! 由 JobBase 的子类通过 _NotifyProgress 激发
        FTLINLINE virtual void OnJobProgress(LONG nJobIndex , CFJobBase<T>* pJob, LONGLONG nCurPos, LONGLONG nTotalSize)
        {
            UNREFERENCED_PARAMETER(nJobIndex);
            UNREFERENCED_PARAMETER(pJob);
            UNREFERENCED_PARAMETER(nCurPos);
            UNREFERENCED_PARAMETER(nTotalSize);
        }
        //! 由 JobBase 的子类通过 _NotifyError 激发
        FTLINLINE virtual void OnJobError(LONG nJobIndex , CFJobBase<T>* pJob, DWORD dwError, LPCTSTR pszDescription)
        {
            UNREFERENCED_PARAMETER(nJobIndex);
            UNREFERENCED_PARAMETER(pJob);
            UNREFERENCED_PARAMETER(dwError);
            UNREFERENCED_PARAMETER(pszDescription);
            //FTLTRACEEX(FTL::tlError, TEXT("IFThreadPoolCallBack[0x%x]::OnJobError[%d], dwError=%d, pszDesc=%s\n"), 
            //    this, nJobIndex, dwError, pszDescription);
            ATLTRACE(TEXT("IFThreadPoolCallBack[0x%x]::OnJobError[%d], dwError=%d, pszDesc=%s\n"), 
                this, nJobIndex, dwError, pszDescription);
        }
    };

    //! Pool中工作线程获取Job时的各种返回值
    enum GetJobType
    {
        typeStop,               //! 获取Job时整个Pool被停止，线程将退出
        typeSubtractThread,     //! 需要减少一个线程，返回值为该值的线程将退出
        typeGetJob,             //! 获得一个Job，返回值为该值的线程将运行获得的Job
        typeError,              //! 发生未知错误 -- 目前尚不清楚什么情况下会发生(可能相关句柄被强制关掉?)
    };

    FTLEXPORT template <typename T>  
    class CFThreadPool
    {
        //typedef CFSharePtr<CFJobBase< T> > CFJobBasePtr;
        friend class CFJobBase<T>;  //允许Job在 GetJobWaitType 中获取 m_hEventStop
        DISABLE_COPY_AND_ASSIGNMENT(CFThreadPool);
    public:
        //! @brief Pool的构造函数
        //! @param [in] pCallBack 状态回调的指针，为了简化起见，只能在构造中设置一个，且设置后不能更改(由业务代码来保证生存期)
        //! @param [in] nMaxWaitingJobs 等待队列中最多能放置多少Job，超过该数值后，Submit会阻塞或失败直到有Job运行完
        FTLINLINE CFThreadPool(IFThreadPoolCallBack<T>* pCallBack = NULL, LONG nMaxWaitingJobs = LONG_MAX);

        FTLINLINE virtual ~CFThreadPool(void);

        //! 开始线程池,此时会创建 nMinNumThreads 个线程，然后会根据任务数在 nMinNumThreads 到 nMaxNumThreads 之间自行调节线程的个数
        FTLINLINE BOOL Start(LONG nMinNumThreads, LONG nMaxNumThreads);

        //! 返回线程池中线程数的调节范围
        FTLINLINE BOOL GetThreadsCount(LONG* pMinNumThreads, LONG* pMaxNumThreads) const;

        //! 动态调整线程个数，如果参数小于0，表示不改变对应的线程数
        //! @note 当设置的Max线程个数小于当前已经运行的线程个数时，不会自动停止多余的Job。当Job结束后，多余的线程就会自动退出。
        FTLINLINE BOOL SetThreadsCount(LONG nMinNumThreads, LONG nMaxNumThreads);

        //! 请求停止线程池
        //! @note 1.只是设置StopEvent，需要Job子类根据GetJobWaitType处理，才能真正的停止
        //! @note 2.不会清除当前注册的但尚未进行的工作，如果需要删除，需要调用ClearUndoWork(析构函数中会调用)
        FTLINLINE BOOL Stop();

        //! 停止并等待线程池结束
        FTLINLINE BOOL StopAndWait(DWORD dwTimeOut = FTL_MAX_THREAD_DEADLINE_CHECK);

        //! 等待所有线程都结束并释放Start中分配的线程资源
        FTLINLINE BOOL Wait(DWORD dwTimeOut = FTL_MAX_THREAD_DEADLINE_CHECK);

        //! 清除当前未进行的工作
        FTLINLINE BOOL ClearUndoWork();
        
        //! 获取当前线程池的运行状态，返回当前正在运行的Job和等待运行的Job个数
        FTLINLINE BOOL GetRunningStatus(INT* pDoingJobCount, INT* pWaitingJobCount);

        //! @brief 向线程池中注册工作 -- 如果当前没有空闲的线程，并且当前线程数小于最大线程数，则会自动创建新的线程，
        //!        成功后会通过 pOutJobIndex 返回Job的索引号，可通过该索引定位操作特定的Job.
        //! @return 返回TRUE表示成功加入Job，返回FALSE表示失败，需要通过 GetLastError 获取详细的信息。
        //!   @retval ERROR_CANCELLED -- 增加Job时线程池停止
        //!   @retval ERROR_TIMEOUT -- 超时
        //! @warning 如果返回FALSE，调用方需要释放 pJob 相关的资源
        //! @todo 本考虑在失败时由 Pool 调用 pJob->OnFinalize(TRUE) 来释放资源，但考虑到重复性检测(虽然该功能现在已经删除)的问题，觉得还是由调用方释放比较合理
        //! @todo 考虑加入 bWorkImmediately -- 是否立即工作，如果是TRUE的话，当当前线程池中没有空闲线程时，会强制创建一个线程
        FTLINLINE BOOL SubmitJob(CFJobBase<T>* pJob, LONG* pOutJobIndex, DWORD dwMilliseconds = INFINITE); //, BOOL bWorkImmediately = FALSE);

        //! 暂停指定的Job，需要Job子类在业务逻辑中周期性调用 GetJobWaitType 来支持
        //! @note: 如果取出Job给客户，可能调用者得到指针时，Job执行完毕 delete this，会造成野指针异常
        FTLINLINE BOOL PauseJob(LONG nJobIndex);

        //! 恢复指定Job运行
        FTLINLINE BOOL ResumeJob(LONG nJobIndex);

        //! 取消指定的Job,如果Job尚未运行，则直接清除该Job；如果正在运行，则调用其 _Cancel 方法；如果Job已经运行完毕，则直接返回TRUE
        FTLINLINE BOOL CancelJob(LONG nJobIndex);

        //! 获取Job优先级
        FTLINLINE BOOL GetJobPriority(LONG nJobIndex, LONG& nJobPriority);

        //! 动态更改Job的优先级,如果Job尚未运行，则会调整在等待队列中的位置；如果Job正在运行，简单更改优先值的数值
        FTLINLINE BOOL SetJobPriority(LONG nJobIndex, LONG nNewPriority);

        //! 请求暂停线程池的操作
        FTLINLINE BOOL PauseAll();

        //! 请求继续线程池的操作
        FTLINLINE BOOL ResumeAll();

        //! 是否已经请求了停止线程池
        FTLINLINE BOOL HadRequestStop() const;
    protected:
        //! 增加运行的线程,如果 当前线程数 + nThreadNum <= m_nMaxNumThreads 时 会成功执行
        FTLINLINE BOOL _AddJobThread(LONG nThreadNum);
        FTLINLINE void _DestroyPool();
        FTLINLINE void _DoJobs();

        FTLINLINE GetJobType _GetJob(CFJobBase<T>** ppJob);

        FTLINLINE void _NotifyJobBegin(CFJobBase<T>* pJob);
        FTLINLINE void _NotifyJobEnd(CFJobBase<T>* pJob);
        FTLINLINE void _NotifyJobCancel(CFJobBase<T>* pJob);

        FTLINLINE void _NotifyJobProgress(CFJobBase<T>* pJob, LONGLONG nCurPos, LONGLONG nTotalSize);
        FTLINLINE void _NotifyJobError(CFJobBase<T>* pJob, DWORD dwError, LPCTSTR pszDescription); 

        typedef BOOL (CALLBACK CFThreadPool::*HandleJobProc)(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
        FTLINLINE BOOL _FindAndHandleSpecialJob(LONG nJobIndex, HandleJobProc pProc, LONG_PTR param);
        FTLINLINE BOOL CALLBACK _InnerPauseJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
        FTLINLINE BOOL CALLBACK _InnerResumeJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
        FTLINLINE BOOL CALLBACK _InnerCancelJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
        FTLINLINE BOOL CALLBACK _InnerGetJobPriority(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
        FTLINLINE BOOL CALLBACK _InnerSetJobPriority(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param);
    protected:
        //! 线程池中最少的线程个数
        LONG m_nMinNumThreads;

        //! 线程池中最大的线程个数
        LONG m_nMaxNumThreads;

        //! 等待队列中的最大线程个数
        LONG m_nMaxWaitingJobs;

        //! 回调接口
        IFThreadPoolCallBack<T>* m_pCallBack;

        //! Job的索引，每 SubmitJob 一次，则递增1，若溢出则重新开始
        LONG m_nJobIndex;

        //! 当前正在运行的Job个数
        LONG m_nRunningJobNumber;

        //! 当前运行着的线程个数(用来在所有的线程结束时激发 Complete 事件)
        LONG m_nRunningThreadNum;

        typedef std::map<DWORD, HANDLE>   TaskThreadContrainer;
        //! 管理工作线程句柄的容器，采用 ThreadId -> Handle 的映射
        TaskThreadContrainer m_TaskThreads;

        //! 保存等待Job的信息，由于有优先级的问题，而且一般是从最前面开始取，因此保存成按照 WaitingJobSorter 排序的map
        typedef std::map<WaitingJobSorter, CFJobBase<T> * > WaitingJobContainer;
        typedef std::map<LONG, CFJobBase<T>* >	IndexToJobContainer;

        //! 等待运行的Job,按照 WaitingJobSorter 排序的 map
        //! @see m_WaitingIndexJobs
        //! @note 两个容器里面保存的Job指针是一样的，只是查询方式不一样
        WaitingJobContainer		m_WaitingJobs;
        //! 等待运行的Job，按照 JobIndex 排序的 map
        //! @see m_WaitingJobs
        //! @note 两个容器里面保存的Job指针是一样的，只是查询方式不一样
        IndexToJobContainer		m_WaitingIndexJobs;

        //! 正在运行的Job，由于会频繁加入、删除，且需要按照JobIndex查找，因此保存成按 JobIndex 查找的 map
        IndexToJobContainer		m_DoingJobs;

        //! 停止Pool的事件
        HANDLE m_hEventStop;

        //! 所有的线程都结束时激发这个事件
        //! @warning 该事件激发时只代表最后一个Job线程将退出线程池的Job执行函数，不代表真能退出(比如业务逻辑代码造成线程退出时死锁?),
        //!          这个时候 Wait 函数会通过断言方式提示开发人员，开发人员需要自行解决这些问题。
        HANDLE m_hEventAllThreadComplete;

        //! 保存等待容器中还可以放的Job个数，每取出一个Job就增加1，每Submit一个进去就减1
        HANDLE m_hSemaphoreWaitingPos;

        //! 保存还有多少个Job的信号量,每Submit一个Job,就增加一个
        HANDLE m_hSemaphoreJobToDo;

        //! 用于减少线程个数时的信号量,初始时个数为0,每要释放一个，就增加一个，
        HANDLE m_hSemaphoreSubtractThread;

        //! 访问 m_DoingJobs 时互斥
        CFCriticalSection m_lockDoingJobs;

        //! 访问 m_WaitingJobs 时互斥
        CFCriticalSection m_lockWaitingJobs;

        //! 访问 m_pJobThreadHandles/m_pJobThreadIds 时互斥
        CFCriticalSection m_lockThreads;

        //! 工作线程的执行函数
        static unsigned int CALLBACK JobThreadProc(void *pParam);
    };
}

#endif //FTL_THREADPOOL_H

#ifndef USE_EXPORT
#  include "ftlThreadPool.hpp"
#endif