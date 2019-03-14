#pragma once
#include <kutil/memory.h>
namespace msddk { ;

enum EventType
{
	kManualResetEvent,
	kAutoResetEvent,
};

class CKeEvent:NonPagedObject
{
private:
	KEVENT * m_pEvent;
public:
	CKeEvent(EVENT_TYPE Type, bool State = false)
	{
		m_pEvent = (KEVENT *)npagednew(sizeof(KEVENT));
		KeInitializeEvent(m_pEvent, Type, State);
	}
public:
	CKeEvent(bool initialState = false, EventType eventType = kManualResetEvent)
	{
		m_pEvent = (KEVENT *)npagednew(sizeof(KEVENT));
		C_ASSERT(NotificationEvent == kManualResetEvent);
		C_ASSERT(SynchronizationEvent == kAutoResetEvent);
		KeInitializeEvent(m_pEvent, (EVENT_TYPE)eventType, initialState);
	}

	~CKeEvent()
	{
		delete m_pEvent;
	}

	bool Valid()
	{
		return (m_pEvent != NULL);
	}

	void Set()
	{
		SetEx();
	}

	void Reset()
	{
		ASSERT(Valid());
		KeClearEvent(m_pEvent);
	}

	bool IsSet()
	{
		ASSERT(Valid());
		return (KeReadStateEvent(m_pEvent) != FALSE);
	}

	void Wait()
	{
		WaitEx();
	}

	bool TryWait(unsigned timeoutInMilliseconds = 0)
	{
		return WaitWithTimeoutEx(((LONGLONG)timeoutInMilliseconds) * 10000) == STATUS_SUCCESS;
	}

public:
	bool SetEx(KPRIORITY Increment = IO_NO_INCREMENT, bool NextCallIsWaitXXX = false)
	{
		ASSERT(Valid());
		return (KeSetEvent(m_pEvent, Increment, NextCallIsWaitXXX) != FALSE);
	}

	NTSTATUS WaitEx(KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		ASSERT(Valid());
		ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
		return KeWaitForSingleObject(m_pEvent, WaitReason, WaitMode, Alertable, NULL);
	}

	NTSTATUS WaitWithTimeoutEx(LONGLONG Timeout,
		KWAIT_REASON WaitReason = Executive,
		KPROCESSOR_MODE WaitMode = KernelMode,
		bool Alertable = false)
	{
		
		ASSERT(Valid());
		LARGE_INTEGER liTimeout;
		liTimeout.QuadPart = Timeout;
		return KeWaitForSingleObject(m_pEvent, WaitReason, WaitMode, Alertable, &liTimeout);
	}
	operator PKEVENT()
	{
		ASSERT(Valid());
		return m_pEvent;
	}
};
};