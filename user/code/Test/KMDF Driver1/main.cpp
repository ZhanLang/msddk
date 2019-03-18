


#include <kprocess/porcapi.h>
#include <wdm.h>
#include <kframe/frame.h>
#include <kframe/simple_drive.h>
#include <kframe/unknown_device.h>
#include <klpc/client.h>
#include <klog/log.h>
#include <klog/logworker.h>
#include <ktime/time.h>
#include <kthread/member_thread.h>
#include <kthread/workitem.h>
#include <kutil/vector.h>
#include <kutil/map.h>
#include <kutil/strconv.h>
#include <jansson/jansson.h>
#include <jansson/jansson.hpp>
#include <kutil/memory.hpp>

using namespace msddk;
CKeLogWorker* pLogWorker;
class MyDevicer : public CUnknownDevcie 
{
public:
	MyDevicer()
	{
		
		m_pObHandle = NULL;
	}
	virtual ~MyDevicer()
	{

	}


	virtual NTSTATUS OnAfterCreate()
	{
		NTSTATUS st = STATUS_SUCCESS;
		KdPrint(("OnAfterCreate"));
		const char* json = "{ \"name\":\"maguojun\",\"arge\":100 }";
		json_error_t error;
		json_t* p = json_loads(json, 0, &error);

		const char *key;
		json_t *value;
		json_object_foreach(p, key, value)
		{
			KdPrint(("key:%s\n", key));
			if (json_is_string(value))
			{
				const char* v = json_string_value(value);
				KdPrint(("value:%s\n", v));
			}
			if ( json_is_integer(value))
			{
				int i = json_integer_value(value);
				KdPrint(("value:%d\n", i));
			}
		}
		json_decref(p);


		json_t* ob = json_object();
		value = json_string("maguojun123");
		json_object_set(ob, "name", value);
		json_decref(value);

		CKeStringA sss = u2utf8(L"Âí¹ú¾ü");
		KdPrint(("u2multi:%s\n", sss.GetBuffer()));
		value = json_string(sss);
		json_object_set(ob, "xin", value);
		json_decref(value);

		value = json_integer(1000);
		json_object_set(ob, "nianling", value);
		json_decref(value);

		char* pchar = json_dumps(ob, 0);
		KdPrint(("ob:%s\n", pchar));
		json_decref(ob);

		CKeStringW s = utf82u(pchar);
		KdPrint(("a2u:%ws", s.GetBuffer()));
		CKeStringA sa = u2a(s);
		KdPrint(("u2a:%s", sa.GetBuffer()));

		jsonp_free(pchar);
		return st;
	}

	virtual NTSTATUS OnBeforeDelete()
	{
		//delete strVector;
		return STATUS_SUCCESS;
	}


private:
	CKeVector<DWORD,VectorNonPagePool> *strVector;
	CKeMap<DWORD, CKeStringW, NonPagedObject>* Map;
	PVOID m_pObHandle;
};

DRIVER_ENTRY(SimpleDrive<MyDevicer>);