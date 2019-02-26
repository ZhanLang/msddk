#ifndef FTL_THREADPOOL_HPP
#define FTL_THREADPOOL_HPP
#pragma once

#ifdef USE_EXPORT
#  include "ftlThreadPool.h"
#endif

namespace FTL
{
    ///////////////////////////////////////////// CFJobBase ///////////////////////////////////////////////////
    template <typename T>
    CFJobBase<T>::CFJobBase(BOOL bSuspendOnCreate /* = FALSE */)
        :m_JobParam(T())		//��ʼ��
    {
        m_bSuspendOnCreate = bSuspendOnCreate;
        m_nJobPriority = 0;
        m_nJobIndex = 0;
        m_hEventJobContinue = NULL;
        m_hEventJobStop = NULL;
        m_dwErrorStatus = ERROR_SUCCESS;
        //m_JobStatus = jsWaiting;
        m_pThreadPool = NULL;
    }
    
    template <typename T>
    CFJobBase<T>::CFJobBase(const T& rJobParam)
        :m_JobParam(rJobParam)
    {
        m_bSuspendOnCreate = rJobParam.m_bSuspendOnCreate;
        m_nJobPriority = 0;
        m_nJobIndex = 0;
        m_hEventJobContinue = NULL;
        m_hEventJobStop = NULL;
        m_dwErrorStatus = ERROR_SUCCESS;
        //m_JobStatus = jsWaiting;
        m_pThreadPool = NULL;
    }

    template <typename T>
    CFJobBase<T>::~CFJobBase()
    {
        //��֤�ں���Դ��δ������Ѿ��ͷ����
        FTLASSERT(NULL == m_hEventJobContinue);
        FTLASSERT(NULL == m_hEventJobStop);
    }

    template <typename T>
    LONG CFJobBase<T>::SetPriority(LONG nNewPriority)
    {
        LONG nOldPriority = m_nJobPriority;
        m_nJobPriority = nNewPriority;
        return nOldPriority;
    }

    template <typename T>
    LONG CFJobBase<T>::GetJobIndex() const
    {
        return m_nJobIndex;
    }
    
    template <typename T>
    DWORD CFJobBase<T>::GetErrorStatus() const
    {
        return m_dwErrorStatus;
    }

    template <typename T>
    LPCTSTR CFJobBase<T>::GetErrorInfo() const
    {
        return m_strFormatErrorInfo.GetString();
    }

    template <typename T>
    BOOL CFJobBase<T>::_Pause()
    {
        BOOL bRet = TRUE;
        m_bSuspendOnCreate = TRUE;
        if (m_hEventJobContinue)
        {
            API_VERIFY(ResetEvent(m_hEventJobContinue));
        }
        return bRet;
    }

    template <typename T>
    BOOL CFJobBase<T>::_Resume()
    {
        BOOL bRet = TRUE;
        m_bSuspendOnCreate = FALSE;
        if (m_hEventJobContinue)
        {
            API_VERIFY(SetEvent(m_hEventJobContinue));	
        }
        return bRet;
    }

    template <typename T>
    BOOL CFJobBase<T>::_Cancel()
    {
        BOOL bRet = TRUE;
        API_VERIFY(SetEvent(m_hEventJobStop));
        return bRet;
    }

    template <typename T>
    void CFJobBase<T>::_SetErrorStatus(DWORD dwErrorStatus, LPCTSTR pszErrorInfo)
    {
        m_dwErrorStatus = dwErrorStatus;
        if (pszErrorInfo)
        {
            m_strFormatErrorInfo.Format(TEXT("%s"), pszErrorInfo);
        }
    }

    template <typename T>
    void CFJobBase<T>::_NotifyProgress(LONGLONG nCurPos, LONGLONG nTotalSize)
    {
        FTLASSERT(m_pThreadPool);
        m_pThreadPool->_NotifyJobProgress(this, nCurPos, nTotalSize);
    }

    template <typename T>
    void CFJobBase<T>::_NotifyCancel()
    {
        m_pThreadPool->_NotifyJobCancel(this);
    }

    template <typename T>
    void CFJobBase<T>::_NotifyError()
    {
        m_pThreadPool->_NotifyJobError(this, m_dwErrorStatus, m_strFormatErrorInfo);
    }

    template <typename T>
    void CFJobBase<T>::_NotifyError(DWORD dwError, LPCTSTR pszDescription)
    {
        //TODO:Need _SetErrorStatus ?
        m_pThreadPool->_NotifyJobError(this, dwError, pszDescription);
    }

    template <typename T>
    FTLThreadWaitType CFJobBase<T>::GetJobWaitType(DWORD dwMilliseconds /* = INFINITE*/) const
    {
        FUNCTION_BLOCK_TRACE(0);
        FTLThreadWaitType waitType = ftwtError;
        HANDLE waitEvent[] = 
        {
            m_hEventJobStop,
            m_pThreadPool->m_hEventStop,
            m_hEventJobContinue
        };
        DWORD dwResult = ::WaitForMultipleObjects(_countof(waitEvent), waitEvent, FALSE, dwMilliseconds);
        switch (dwResult)
        {
        case WAIT_OBJECT_0:
            waitType = ftwtStop;		//Job Stop Event
            break;
        case WAIT_OBJECT_0 + 1:
            waitType = ftwtStop;		//Thread Pool Stop Event
            break;
        case WAIT_OBJECT_0 + 2:
            waitType = ftwtContinue;	//Job Continue Event
            break;
        case WAIT_TIMEOUT:
            waitType = ftwtTimeOut;
            break;
        default:
            FTLASSERT(FALSE);			//how?
            waitType = ftwtError;
            break;
        }
        return waitType;
    }

    //////////////////////////////////////    CFThreadPool    ///////////////////////////////////////////////////
    template <typename T>  
    CFThreadPool<T>::CFThreadPool(IFThreadPoolCallBack<T>* pCallBack /* = NULL*/, LONG nMaxWaitingJobs /* = LONG_MAX */)
        :m_pCallBack(pCallBack)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        
        m_nMaxWaitingJobs = nMaxWaitingJobs;
        m_nMinNumThreads = 0;
        m_nMaxNumThreads = 1;

        m_nRunningJobNumber = 0;
        m_nJobIndex = 0;
        m_nRunningThreadNum = 0;

        m_hEventStop = CreateEvent(NULL, TRUE, FALSE, NULL);
        FTLASSERT(NULL != m_hEventStop);

        m_hEventAllThreadComplete = ::CreateEvent(NULL, TRUE, TRUE, NULL);
        FTLASSERT(NULL != m_hEventAllThreadComplete);

        m_hSemaphoreWaitingPos = CreateSemaphore(NULL, nMaxWaitingJobs, nMaxWaitingJobs, NULL);
        FTLASSERT(m_hSemaphoreWaitingPos);

        m_hSemaphoreJobToDo = ::CreateSemaphore(NULL, 0, MAXLONG, NULL); //nMaxWaitingJobs
        FTLASSERT(NULL != m_hSemaphoreJobToDo);

        //���������̸߳������ź���
        m_hSemaphoreSubtractThread = CreateSemaphore(NULL, 0, MAXLONG, NULL);
        FTLASSERT(NULL != m_hSemaphoreSubtractThread);
    }

    template <typename T>  
    CFThreadPool<T>::~CFThreadPool(void)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        BOOL bRet = FALSE;
        API_VERIFY(StopAndWait(FTL_MAX_THREAD_DEADLINE_CHECK));
        _DestroyPool();

        FTLASSERT(m_WaitingIndexJobs.empty());
        FTLASSERT(m_WaitingJobs.empty());
        FTLASSERT(m_DoingJobs.empty());

        FTLASSERT(0 == m_nRunningThreadNum);  //����ʱ���е��̶߳�Ҫ����
    }

    template <typename T>  
    BOOL CFThreadPool<T>::Start(LONG nMinNumThreads, LONG nMaxNumThreads)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        FTLASSERT( 0 <= nMinNumThreads );
        FTLASSERT( nMinNumThreads <= nMaxNumThreads );  
        ATLTRACE(TEXT("CFThreadPool::Start, ThreadNum is [%d-%d]\n"), nMinNumThreads, nMaxNumThreads);

        if (nMinNumThreads > nMaxNumThreads
            || nMinNumThreads < 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        BOOL bRet = TRUE;
        m_nMinNumThreads = nMinNumThreads;
        m_nMaxNumThreads = nMaxNumThreads;

        API_VERIFY(ResetEvent(m_hEventStop));
        {
            CFAutoLock<CFLockObject>   locker(&m_lockThreads);
            _AddJobThread(m_nMinNumThreads);		//��ʼʱֻ���� m_nMinNumThreads ���߳�
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::GetThreadsCount(LONG* pMinNumThreads, LONG* pMaxNumThreads) const
    {
        if (!pMinNumThreads && !pMaxNumThreads)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        CFAutoLock<CFLockObject>   locker(&m_lockThreads);
        if (pMinNumThreads)
        {
            *pMinNumThreads = m_nMinNumThreads;
        }
        if (pMaxNumThreads)
        {
            *pMaxNumThreads = m_nMaxNumThreads;
        }
        return TRUE;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::SetThreadsCount(LONG nMinNumThreads, LONG nMaxNumThreads)
    {
        ATLTRACE(TEXT("CFThreadPool<T>::SetThreadsCount, Min:%d=>%d, Max:%d=>%d\n"),
            m_nMinNumThreads, nMinNumThreads, m_nMaxNumThreads, nMaxNumThreads);

        if (nMinNumThreads < 0)
        {
            nMinNumThreads = m_nMinNumThreads;
        }
        if (nMaxNumThreads < 0)
        {
            nMaxNumThreads = m_nMaxNumThreads;
        }

        FTLASSERT(nMinNumThreads <= nMaxNumThreads);
        if (nMinNumThreads > nMaxNumThreads)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        BOOL bRet = TRUE;
        LONG nWaitingJobCount = 0;
        LONG nDoingJobCount = 0;
        LONG nCurrentRunningThreadCount = m_nRunningThreadNum;
        {
            CFAutoLock<CFLockObject> lockerWaiting(&m_lockWaitingJobs);
            nWaitingJobCount = (LONG)m_WaitingJobs.size();
        }
        {
            CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);
            nDoingJobCount = (LONG)m_DoingJobs.size();
        }

        //����������Ӧ����Ҫ���ٸ��߳�
        LONG nNeedRunningThreadCount = FTL_CLAMP(nWaitingJobCount + nDoingJobCount, nMinNumThreads, nMaxNumThreads);
        
        //�Ƚϵ�ǰ���̸߳�������������Ҫ�ĸ�����Ȼ�������Ҫ�������µ��̻߳������ͷ��߳�
        LONG nDiffThreadCount = nNeedRunningThreadCount - nCurrentRunningThreadCount;

        ATLTRACE(TEXT("SetThreadsCount, nNeedRunningThreadCount=%d, nCurrentRunningThreadCount=%d, nDiffThreadCount=%d\n"),
            nNeedRunningThreadCount, nCurrentRunningThreadCount, nDiffThreadCount);

        m_nMinNumThreads = nMinNumThreads;
        m_nMaxNumThreads = nMaxNumThreads;
        if (nDiffThreadCount > 0)
        {
            //��Ҫ�����µ��߳�
            API_VERIFY(_AddJobThread(nDiffThreadCount));
        }
        else if(nDiffThreadCount < 0)
        {
            //��Ҫ�ͷ��߳�
            API_VERIFY(ReleaseSemaphore(m_hSemaphoreSubtractThread, -nDiffThreadCount, NULL));    
        }
        else
        {
            //��ǰ�̸߳��������ʣ��򲻽����κδ���
            bRet = TRUE;
        }

        return bRet;
    }
    
    template <typename T>  
    BOOL CFThreadPool<T>::Stop()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        ATLTRACE(TEXT("CFThreadPool::Stop\n"));

        BOOL bRet = TRUE;
        API_VERIFY(SetEvent(m_hEventStop));
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::StopAndWait(DWORD dwTimeOut /* = FTL_MAX_THREAD_DEADLINE_CHECK */)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        BOOL bRet = TRUE;
        API_VERIFY(Stop());
        API_VERIFY(Wait(dwTimeOut));
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::Wait(DWORD dwTimeOut /* = FTL_MAX_THREAD_DEADLINE_CHECK */)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        ATLTRACE(TEXT("CFThreadPool::Wait, dwTimeOut=%d\n"), dwTimeOut);

        BOOL bRet = TRUE;
        DWORD dwResult = WaitForSingleObject(m_hEventAllThreadComplete, dwTimeOut);
        switch (dwResult)
        {
        case WAIT_OBJECT_0: //���е��̶߳�������
            bRet = TRUE;
            break;
        case WAIT_TIMEOUT:
            ATLTRACE(TEXT("!!!CFThreadPool::Wait, Not all thread over in %d millisec\n"), dwTimeOut);
            FTLASSERT(FALSE && TEXT("CFThreadPool::Wait TimeOut"));
            SetLastError(ERROR_TIMEOUT);
            bRet = FALSE;
            break;
        default:
            FTLASSERT(FALSE);
            bRet = FALSE;
            break;
        }

        {
            CFAutoLock<CFLockObject> locker(&m_lockThreads);
            for (TaskThreadContrainer::iterator iter = m_TaskThreads.begin();
                iter != m_TaskThreads.end(); ++iter)
            {
#ifdef FTL_DEBUG
                //ע�⣺��������ط���������ʾJob�߳������������ˣ�ֻ�������һ��Job�̡߳��Ϳ��˳�JobThreadProc��������
                //      Ϊ�˼�ʱ����ҵ���߼���������߳������Ĵ���(�� http://support.microsoft.com/kb/322909 ��),
                //      ���԰汾ʱ�������ȴ��߳̽���������ͨ���������ѱ�����Ա�����ص�����

                DWORD dwWaitThread = WaitForSingleObject(iter->second, FTL_MAX_THREAD_DEADLINE_CHECK);
                if (WAIT_OBJECT_0 != dwWaitThread)
                {
                    FTLTRACEEX(tlError, TEXT("ERROR: Wait For Job Thread[Id=%d(0x%x), Handle=0x%x] Time Out, dwWaitThread=%d\n"),
                        iter->first, iter->first, iter->second, dwWaitThread);
                }
                FTLASSERT(WAIT_OBJECT_0 == dwWaitThread && TEXT("Wait For Job Thread Time Out, Maybe Deadlock"));
#endif 
                SAFE_CLOSE_HANDLE(iter->second, NULL);
                //API_VERIFY(CloseHandle(iter->second));
            }
            m_TaskThreads.clear();
        }
        
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::PauseAll()
    {
        ATLTRACE(TEXT("CFThreadPool::PauseAll\n"));
        BOOL bRet = TRUE;
        {
            CFAutoLock<CFLockObject> lockerWaiting(&m_lockWaitingJobs);
            for(WaitingJobContainer::iterator iterWaiting = m_WaitingJobs.begin();
                iterWaiting != m_WaitingJobs.end();
                ++iterWaiting)
            {
                CFJobBase<T>* pJob = iterWaiting->second;
                API_VERIFY(pJob->_Pause());
            }
        }
        {
            CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);
            for (IndexToJobContainer::iterator iterDoing = m_DoingJobs.begin();
                iterDoing != m_DoingJobs.end();
                ++iterDoing)
            {
                CFJobBase<T>* pJob = iterDoing->second;
                API_VERIFY(pJob->_Pause());
            }
        }
        //API_VERIFY(::ResetEvent(m_hEventContinue));
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::ResumeAll()
    {
        ATLTRACE(TEXT("CFThreadPool::ResumeAll\n"));
        BOOL bRet = TRUE;
        {
            CFAutoLock<CFLockObject> lockerWaiting(&m_lockWaitingJobs);
            for(WaitingJobContainer::iterator iterWaiting = m_WaitingJobs.begin();
                iterWaiting != m_WaitingJobs.end();
                ++iterWaiting)
            {
                CFJobBase<T>* pJob = iterWaiting->second;
                API_VERIFY(pJob->_Resume());
            }
        }
        {
            CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);
            for (IndexToJobContainer::iterator iterDoing = m_DoingJobs.begin();
                iterDoing != m_DoingJobs.end();
                ++iterDoing)
            {
                CFJobBase<T>* pJob = iterDoing->second;
                API_VERIFY(pJob->_Resume());
            }
        }
        //API_VERIFY(::SetEvent(m_hEventContinue));
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::ClearUndoWork()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);

        BOOL bRet = TRUE;
        {
            CFAutoLock<CFLockObject> locker(&m_lockWaitingJobs);
            ATLTRACE(TEXT("CFThreadPool::ClearUndoWork, waitingJob Number is %d\n"), m_WaitingJobs.size());
            while (!m_WaitingJobs.empty())
            {
                //��ȡһ����Ӧ���ű���󣬱�֤������� m_WaitingJobs �ĸ�����һ�µ�
                DWORD dwResult = WaitForSingleObject(m_hSemaphoreJobToDo, FTL_MAX_THREAD_DEADLINE_CHECK); 
                API_VERIFY(dwResult == WAIT_OBJECT_0);
                
                //�ͷ�һ���ȴ����п�λ���ű���󣬱�֤���λ�ĸ�����ȷ
                LONG nPreviousCount = 0;
                API_VERIFY(ReleaseSemaphore(m_hSemaphoreWaitingPos, 1, &nPreviousCount));
                FTLASSERT(nPreviousCount == (LONG)(m_nMaxWaitingJobs - m_WaitingJobs.size()));
                UNREFERENCED_PARAMETER(nPreviousCount);

                WaitingJobContainer::iterator iterBegin = m_WaitingJobs.begin();
                CFJobBase<T>* pJob = iterBegin->second;
                FTLASSERT(pJob);
                //ͬʱ�� m_WaitingJobs �� m_WaitingIndexJobs �����Job��Ϣ
                m_WaitingJobs.erase(iterBegin);
                m_WaitingIndexJobs.erase(pJob->GetJobIndex());

                _NotifyJobCancel(pJob);
                pJob->OnFinalize(TRUE);
            }
            FTLASSERT(m_WaitingIndexJobs.empty());
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::GetRunningStatus(INT* pDoingJobCount, INT* pWaitingJobCount)
    {
        //ͬʱ��ס �ȴ����� �� ���ж��У���֤�ڻ�ȡʱû�з��� Job ��Ǩ��
        CFAutoLock<CFLockObject> lockerWaiting(&m_lockWaitingJobs);
        CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);
        if (pWaitingJobCount)
        {
            *pWaitingJobCount = (INT)m_WaitingJobs.size();
            FTLASSERT(m_WaitingIndexJobs.size() == m_WaitingJobs.size());
        }

        if (pDoingJobCount)
        {
            *pDoingJobCount = (INT)m_DoingJobs.size();
        }
        return TRUE;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::_AddJobThread(LONG nThreadNum)
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        BOOL bRet = TRUE;
        {
            CFAutoLock<CFLockObject> locker(&m_lockThreads);
            if ((LONG)m_TaskThreads.size() + nThreadNum > m_nMaxNumThreads)
            {
                FTLASSERT(FALSE); //�����߼������˴��� ?
                //�����������������ټ���
                SetLastError(ERROR_INVALID_PARAMETER);
                bRet = FALSE;
            }
            else
            {
                unsigned int threadId = 0;
                for(LONG i = 0;i < nThreadNum; i++)
                {
                    HANDLE hThread = (HANDLE) _beginthreadex( NULL, 0, JobThreadProc, this, 0, &threadId);
                    FTLASSERT(hThread != NULL);
                    FTLASSERT(m_TaskThreads.find(threadId) == m_TaskThreads.end());
                    m_TaskThreads[threadId] = hThread;

                    ATLTRACE(TEXT("CFThreadPool::_AddJobThread, ThreadId=%d(0x%x), CurNumThreads=%d\n"),
                        threadId, threadId, m_TaskThreads.size());
                }
                bRet = TRUE;
            }
        }
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::SubmitJob(CFJobBase<T>* pJob, LONG* pOutJobIndex, DWORD dwMilliseconds /* = INFINITE */)
    {
        FTLASSERT(NULL != m_hEventStop); //������� _DestroyPool�󣬾Ͳ����ٴε��øú���
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        BOOL bRet = FALSE;
        
        HANDLE hWaitHandles[] = 
        {
            m_hEventStop,
            m_hSemaphoreWaitingPos,
        };

        DWORD dwLastError = 0;
        DWORD dwResult = WaitForMultipleObjects(_countof(hWaitHandles), hWaitHandles, FALSE, dwMilliseconds);
        switch (dwResult)
        {
        case WAIT_OBJECT_0:     //Stop
            dwLastError = ERROR_CANCELLED;
            bRet = FALSE;
            break;
        case WAIT_TIMEOUT:      //There is no place and TimeOut
            dwLastError = ERROR_TIMEOUT;
            bRet = FALSE;
            break;
        case WAIT_OBJECT_0 + 1: //wait for place
            bRet = TRUE;
            break;
        default:
            FTLASSERT(FALSE);   //Why?
            bRet = FALSE;
            dwLastError = GetLastError();
            break;
        }

        //����Jobʧ�ܣ�
        if (!bRet)
        {
            //TODO�������Ƿ�Ӧ�����Job����Դ������˵Ӧ���ɵ�������� -- ���ȷ���ɵ��������
            //_NotifyJobCancel(pJob);
            //pJob->OnFinalize(TRUE);
            SetLastError(dwLastError);
            return bRet;
        }

        //����Job���һ���һ���ȴ��߳�
        {
            CFAutoLock<CFLockObject> locker(&m_lockWaitingJobs);
            m_nJobIndex++;
            if (m_nJobIndex < 0)
            {
                //����� -- ����������˶�õķ������
                m_nJobIndex = 1;
            }
            pJob->m_pThreadPool = this;         //����˽�б����������Լ���ֵ��ȥ
            pJob->m_nJobIndex = m_nJobIndex;	//����˽�б���������JobIndex

            if (pOutJobIndex)
            {
                *pOutJobIndex = pJob->m_nJobIndex;
            }
            
            m_WaitingJobs.insert(WaitingJobContainer::value_type(WaitingJobSorter(pJob->GetPriority(), pJob->GetJobIndex()), pJob));
            m_WaitingIndexJobs.insert(IndexToJobContainer::value_type(pJob->GetJobIndex(), pJob));

            API_VERIFY(ReleaseSemaphore(m_hSemaphoreJobToDo, 1L, NULL));
        }

        SwitchToThread();//���ѵȴ����̣߳�ʹ�������߳̿��Ի�ȡJob -- ע�� CFAutoLock �ķ�Χ

        {
            //�����е��̶߳�������Jobʱ������Ҫ�����߳�  -- ���� m_nRunningJobNumber �ӱ���(ֻ�Ƕ�ȡ)
            CFAutoLock<CFLockObject> locker(&m_lockThreads);
            LONG nCurNumThreads = (LONG)m_TaskThreads.size();
            FTLASSERT(m_nRunningJobNumber <= nCurNumThreads);
            BOOL bNeedMoreThread = (m_nRunningJobNumber == nCurNumThreads) && (nCurNumThreads < m_nMaxNumThreads); 
            if (bNeedMoreThread)
            {
                API_VERIFY(_AddJobThread(1L));      //ÿ������һ���߳�
            }

            ATLTRACE(TEXT("CFThreadPool::SubmitJob, pJob[%d] = 0x%p, m_nRunningJobNumber=%d, m_nCurNumThreads=%d, bNeedMoreThread=%d\n"),
                pJob->m_nJobIndex, pJob, m_nRunningJobNumber, m_TaskThreads.size(), bNeedMoreThread);
        }

        return bRet;	
    }

    template <typename T>
    BOOL CFThreadPool<T>::_FindAndHandleSpecialJob( LONG nJobIndex, HandleJobProc pProc, LONG_PTR param)
    {
        FTLASSERT(pProc);
        if (!pProc)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (nJobIndex <= 0 || nJobIndex > m_nJobIndex)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        BOOL bRet = FALSE;
        BOOL bFoundWaiting = FALSE;
        BOOL bFoundDoing = FALSE;

        {
            //���Ȳ���δ����������
            CFAutoLock<CFLockObject> locker(&m_lockWaitingJobs);
            IndexToJobContainer::iterator iterIndex = m_WaitingIndexJobs.find(nJobIndex);
            if (iterIndex != m_WaitingIndexJobs.end())
            {
                //�ҵ�,˵�����Job��û������
                bFoundWaiting = TRUE;

                CFJobBase<T>* pJob = iterIndex->second;
                FTLASSERT(pJob);
                FTLASSERT(pJob->GetJobIndex() == nJobIndex);

                bRet = (this->*pProc)(pJob, TRUE, param);
            }
        }

        if (!bFoundWaiting)
        {
            //�����������е�����
            CFAutoLock<CFLockObject> locker(&m_lockDoingJobs);
            IndexToJobContainer::iterator iterDoing = m_DoingJobs.find(nJobIndex);
            if (iterDoing != m_DoingJobs.end())
            {
                bFoundDoing = TRUE;

                CFJobBase<T>* pJob = iterDoing->second;
                FTLASSERT(pJob);
                FTLASSERT(pJob->GetJobIndex() == nJobIndex);
                
                bRet = (this->*pProc)(pJob, FALSE, param);
            }
        }

        if (!bFoundWaiting && !bFoundDoing)
        {
            //Waiting �� Doing �ж�û���ҵ����Ѿ�ִ�����
            bRet = TRUE;
        }

        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::_InnerPauseJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param)
    {
        UNREFERENCED_PARAMETER(bFoundInWaiting);
        UNREFERENCED_PARAMETER(param);
        FTLASSERT(pJob);
        BOOL bRet = TRUE;
        API_VERIFY(pJob->_Pause());
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::_InnerResumeJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param)
    {
        UNREFERENCED_PARAMETER(bFoundInWaiting);
        UNREFERENCED_PARAMETER(param);
        FTLASSERT(pJob);
        BOOL bRet = TRUE;
        API_VERIFY(pJob->_Resume());
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::_InnerCancelJob(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param)
    {
        UNREFERENCED_PARAMETER(param);
        BOOL bRet = FALSE;
        FTLASSERT(pJob);
        if (bFoundInWaiting)
        {
            DWORD dwResult = WaitForSingleObject(m_hSemaphoreJobToDo, INFINITE); //�ͷŶ�Ӧ���ű���󣬱��������ƥ��
            //API_ASSERT(dwResult == WAIT_OBJECT_0);
            UNREFERENCED_PARAMETER(dwResult);

            WaitingJobSorter tmpCancelJob(pJob->GetPriority(), pJob->GetJobIndex());
            WaitingJobContainer::iterator iterWaiting = m_WaitingJobs.find(tmpCancelJob);
            FTLASSERT(iterWaiting != m_WaitingJobs.end());
            m_WaitingJobs.erase(iterWaiting);
            m_WaitingIndexJobs.erase(pJob->GetJobIndex());

            _NotifyJobCancel(pJob);
            pJob->OnFinalize(TRUE);
            bRet = TRUE;
        }
        else
        {
            //ע�⣺����ֻ������Cancel��ʵ���������Ƿ�������Cancel����Ҫ����Job��ʵ�֣�
            bRet = pJob->_Cancel();
            //��Ҫ m_DoingJobs.erase(iterDoing) -- Job ������� erase
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::PauseJob(LONG nJobIndex)
    {
        FTLTRACEEX(tlTrace, TEXT("CFThreadPool::PauseJob, JobIndex=%d\n"), nJobIndex);
        BOOL bRet = FALSE;
        {
            bRet = _FindAndHandleSpecialJob(nJobIndex, &CFThreadPool<T>::_InnerPauseJob, NULL);
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::ResumeJob(LONG nJobIndex)
    {
        ATLTRACE(TEXT("CFThreadPool::ResumeJob, JobIndex=%d\n"), nJobIndex);
        BOOL bRet = FALSE;
        {
            bRet = _FindAndHandleSpecialJob(nJobIndex, &CFThreadPool<T>::_InnerResumeJob, NULL);
        }
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::CancelJob(LONG nJobIndex)
    {
        ATLTRACE(TEXT("CFThreadPool::CancelJob, JobIndex=%d\n"), nJobIndex);
        BOOL bRet = FALSE;
        {
            bRet = _FindAndHandleSpecialJob(nJobIndex, &CFThreadPool<T>::_InnerCancelJob, NULL);
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::_InnerGetJobPriority(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param)
    {
        BOOL bRet = FALSE;
        FTLASSERT(pJob);
        LONG *pJobPriority = static_cast<LONG*>(param);
        CHECK_POINTER_WRITABLE_DATA_RETURN_VALUE_IF_FAIL(pJobPriority, sizeof(LONG), FALSE);
        if (pJobPriority)
        {
            *pJobPriority = pJob->GetJobPriority();
            bRet = TRUE;
        }
        return bRet;
    }

    template <typename T>
    BOOL CFThreadPool<T>::_InnerSetJobPriority(CFJobBase<T>* pJob, BOOL bFoundInWaiting, LONG_PTR param)
    {
        BOOL bRet = TRUE;
        LONG *pNewJobPriority = reinterpret_cast<LONG*>(param);
        //CHECK_POINTER_WRITABLE_DATA_RETURN_VALUE_IF_FAIL(pNewJobPriority, sizeof(LONG), FALSE);
        if (pNewJobPriority && (*pNewJobPriority) != pJob->GetPriority())
        {
            if (bFoundInWaiting)
            {
                //������ڵȴ��������ҵ��ģ�����Ҫ�����µ����ȼ����·���ȴ�����

                //_FindAndHandleSpecialJob �����Ѿ��������������˴����Բ�����
                //CFAutoLock<CFLockObject> lockerWating(&m_lockWaitingJobs);  
                WaitingJobSorter tmpSorter(pJob->GetPriority(), pJob->GetJobIndex());
                WaitingJobContainer::iterator iterWaitingJob = m_WaitingJobs.find(tmpSorter);
                FTLASSERT(iterWaitingJob != m_WaitingJobs.end());
                if (iterWaitingJob != m_WaitingJobs.end())
                {
                    //��������ĵȴ����ж�����ɾ���ɵģ������µ����ȼ������¼��룬�Ӷ���֤λ����ȷ
                    m_WaitingJobs.erase(iterWaitingJob);
                    pJob->SetPriority(*pNewJobPriority);
                    m_WaitingJobs.insert(WaitingJobContainer::value_type(WaitingJobSorter(pJob->GetPriority(), pJob->GetJobIndex()), pJob));

                    //��Ϊû�и���Job����������Ϣ����˲����� m_WaitingIndexJobs
                }
            }
            else
            {
                //���ж��� -- �������ȼ��Ѿ�����Ӱ���Ƿ��������У����ֻ�Ǽ򵥵ĸ������ȼ�
                pJob->SetPriority(*pNewJobPriority);
            }
        }
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::GetJobPriority(LONG nJobIndex, LONG& nJobPriority)
    {
        BOOL bRet = _FindAndHandleSpecialJob(nJobIndex, &CFThreadPool<T>::_InnerGetJobPriority, (LONG_PTR)&nJobPriority);
        return bRet;
    }

    template <typename T>  
    BOOL CFThreadPool<T>::SetJobPriority(LONG nJobIndex, LONG nNewPriority)
    {
        ATLTRACE(TEXT("CFThreadPool::SetJobPriority, JobIndex=%d, nNewPriority=%d\n"), nJobIndex, nNewPriority);
        BOOL bRet = _FindAndHandleSpecialJob(nJobIndex, &CFThreadPool<T>::_InnerSetJobPriority, (LONG_PTR)&nNewPriority);
        return bRet;
    }


    template <typename T>  
    BOOL CFThreadPool<T>::HadRequestStop() const
    {
        _ASSERT(NULL != m_hEventStop);
        BOOL bRet = (WaitForSingleObject(m_hEventStop, 0) == WAIT_OBJECT_0);
        return bRet;
    }

    template <typename T>  
    void CFThreadPool<T>::_DestroyPool()
    {
        FUNCTION_BLOCK_TRACE(DEFAULT_BLOCK_TRACE_THRESHOLD);
        BOOL bRet = FALSE;
        API_VERIFY(ClearUndoWork());
        
        SAFE_CLOSE_HANDLE(m_hSemaphoreWaitingPos, NULL);
        SAFE_CLOSE_HANDLE(m_hSemaphoreJobToDo,NULL);
        SAFE_CLOSE_HANDLE(m_hSemaphoreSubtractThread,NULL);
        SAFE_CLOSE_HANDLE(m_hEventAllThreadComplete, NULL);
        SAFE_CLOSE_HANDLE(m_hEventStop,NULL);
    }

    template <typename T>  
    GetJobType CFThreadPool<T>::_GetJob(CFJobBase<T>** ppJob)
    {
        FUNCTION_BLOCK_TRACE(0);
        HANDLE hWaitHandles[] = 
        {
            m_hEventStop,                 //user stop thread pool
            m_hSemaphoreSubtractThread,   //need subtract thread
            m_hSemaphoreJobToDo,          //there are waiting jobs
        };

        DWORD dwResult = WaitForMultipleObjects(_countof(hWaitHandles), hWaitHandles, FALSE, INFINITE);
        switch(dwResult)
        {
        case WAIT_OBJECT_0:				//m_hEventStop
            return typeStop;
        case WAIT_OBJECT_0 + 1:			//m_hSemaphoreSubtractThread
            return typeSubtractThread;
        case WAIT_OBJECT_0 + 2:			//m_hSemaphoreJobToDo
            break;
        default:
            FTLASSERT(FALSE);
            return typeStop;
        }

        {
            //�ӵȴ������л�ȡ�û���ҵ
            CFAutoLock<CFLockObject> lockerWating(&m_lockWaitingJobs);
            FTLASSERT(!m_WaitingJobs.empty());
            WaitingJobContainer::iterator iterBegin = m_WaitingJobs.begin();

            CFJobBase<T>* pJob = iterBegin->second;
            FTLASSERT(pJob);

            *ppJob = pJob;
            m_WaitingJobs.erase(iterBegin);
            m_WaitingIndexJobs.erase(pJob->GetJobIndex());
            {
                ReleaseSemaphore(m_hSemaphoreWaitingPos, 1, NULL);
                //�ŵ�������ҵ��������
                CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);

                FTLASSERT(NULL == pJob->m_hEventJobStop);
                FTLASSERT(NULL == pJob->m_hEventJobContinue);
    
                pJob->m_hEventJobStop = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                pJob->m_hEventJobContinue = ::CreateEvent(NULL, TRUE, !pJob->m_bSuspendOnCreate, NULL);
                //pJob->m_JobStatus = jsDoing;
                m_DoingJobs.insert(IndexToJobContainer::value_type(pJob->GetJobIndex(), pJob));			
            }
        }
        return typeGetJob;	
    }

    template <typename T>  
    void CFThreadPool<T>::_DoJobs()
    {
        BOOL bRet = FALSE;
        //FUNCTION_BLOCK_TRACE(0);
        CFJobBase<T>* pJob = NULL;
        GetJobType getJobType = typeStop;
        while(typeGetJob == (getJobType = _GetJob(&pJob)))
        {
            InterlockedIncrement(&m_nRunningJobNumber);
            INT nJobIndex = pJob->GetJobIndex();
            ATLTRACE(TEXT("CFThreadPool Begin Run Job %d\n"), nJobIndex);

            pJob->GetJobWaitType(INFINITE);	//wait for resume if SuspendOnCreate

            _NotifyJobBegin(pJob);
            pJob->Run();
            _NotifyJobEnd(pJob);

            ATLTRACE(TEXT("CFThreadPool End Run Job %d\n"), nJobIndex);
            {
                //Job���������ȴ������б���ɾ��
                CFAutoLock<CFLockObject> lockerDoing(&m_lockDoingJobs);

                SAFE_CLOSE_HANDLE(pJob->m_hEventJobStop, NULL);
                SAFE_CLOSE_HANDLE(pJob->m_hEventJobContinue, NULL);

                IndexToJobContainer::iterator iter = m_DoingJobs.find(nJobIndex);
                FTLASSERT(m_DoingJobs.end() != iter);
                if (m_DoingJobs.end() != iter)
                {
                    m_DoingJobs.erase(iter);
                }
            }
            
            pJob->OnFinalize(FALSE);

            InterlockedDecrement(&m_nRunningJobNumber);


            //���һ���Ƿ���Ҫ�����߳�
            BOOL bNeedSubtractThread = FALSE;
            {
                CFAutoLock<CFLockObject> lockerWaitingJobs(&m_lockWaitingJobs);
                CFAutoLock<CFLockObject> lockerThreads(&m_lockThreads);
                //��������û��Job�����ҵ�ǰ�߳���������С�߳���ʱ
                bNeedSubtractThread = (m_WaitingJobs.empty() && ((LONG)m_TaskThreads.size() > m_nMinNumThreads) && !HadRequestStop());
                if (bNeedSubtractThread)
                {
                    //֪ͨ����һ���߳�
                    ReleaseSemaphore(m_hSemaphoreSubtractThread, 1L, NULL);
                }
            }
        }
        if (typeSubtractThread == getJobType)  //��Ҫ�����߳�,Ӧ�ð��Լ��˳� -- ע�⣺֪ͨ�˳����̺߳�ʵ���˳����߳̿��ܲ���ͬһ��
        {
            //FUNCTION_BLOCK_NAME_TRACE(TEXT("typeSubtractThread, will remove self thread"),
            //    DEFAULT_BLOCK_TRACE_THRESHOLD);
            CFAutoLock<CFLockObject> locker(&m_lockThreads);
            DWORD dwCurrentThreadId = GetCurrentThreadId();
            {
                FTLASSERT(m_TaskThreads.find(dwCurrentThreadId) != m_TaskThreads.end());

                HANDLE hOldTemp = m_TaskThreads[dwCurrentThreadId];
                m_TaskThreads.erase(dwCurrentThreadId);
                //CloseHandle(hOldTemp);
                SAFE_CLOSE_HANDLE(hOldTemp, NULL);

                ATLTRACE(TEXT("CFThreadPool Subtract a thread, thread id = %d(0x%x), curThreadNum = %d\n"),
                    dwCurrentThreadId, dwCurrentThreadId, m_TaskThreads.size());
            }
        }
        else //typeStop
        {
            //Do Nothing
        }
    }

    template <typename T>  
    void CFThreadPool<T>::_NotifyJobBegin(CFJobBase<T>* pJob)
    {
        FTLASSERT(pJob);
        if (pJob && m_pCallBack)
        {
            m_pCallBack->OnJobBegin(pJob->GetJobIndex(), pJob);
        }
    }

    template <typename T>  
    void CFThreadPool<T>::_NotifyJobEnd(CFJobBase<T>* pJob)
    {
        FTLASSERT(pJob);
        if (pJob && m_pCallBack)
        {
            m_pCallBack->OnJobEnd(pJob->GetJobIndex(), pJob);
        }
    }

    template <typename T>  
    void CFThreadPool<T>::_NotifyJobCancel(CFJobBase<T>* pJob)
    {
        FTLASSERT(pJob);
        if (pJob && m_pCallBack)
        {
            m_pCallBack->OnJobCancel(pJob->GetJobIndex(), pJob);
        }
    }

    template <typename T>  
    void CFThreadPool<T>::_NotifyJobProgress(CFJobBase<T>* pJob, LONGLONG nCurPos, LONGLONG nTotalSize)
    {
        FTLASSERT(pJob);
        if (pJob && m_pCallBack)
        {
            m_pCallBack->OnJobProgress(pJob->GetJobIndex(), pJob, nCurPos, nTotalSize);
        }
    }

    template <typename T>  
    void CFThreadPool<T>::_NotifyJobError(CFJobBase<T>* pJob, DWORD dwError, LPCTSTR pszDescription)
    {
        FTLASSERT(pJob);
        if (pJob && m_pCallBack)
        {
            m_pCallBack->OnJobError(pJob->GetJobIndex(), pJob, dwError, pszDescription);
        }
    }

    template <typename T>  
    unsigned int CFThreadPool<T>::JobThreadProc(void *pThis)
    {
        FUNCTION_BLOCK_TRACE(0);
        BOOL bRet = FALSE;
        CFThreadPool<T>* pThreadPool = (CFThreadPool<T>*)pThis;
        LONG nRunningNumber = InterlockedIncrement(&pThreadPool->m_nRunningThreadNum);
        if (1 == nRunningNumber)
        {
            API_VERIFY(ResetEvent(pThreadPool->m_hEventAllThreadComplete));
        }

        //���� try...catch ���б�������ֹ����ҵ���߼������BUG
        pThreadPool->_DoJobs();

        nRunningNumber = InterlockedDecrement(&pThreadPool->m_nRunningThreadNum);
        if (0 == nRunningNumber)
        {
            //�߳̽������ж��Ƿ������һ���̣߳�����ǣ������¼�
            API_VERIFY(SetEvent(pThreadPool->m_hEventAllThreadComplete));
        }
        return(0);
    }
}

#endif //FTL_THREADPOOL_HPP