#pragma once
#include <kutil/string.h>
namespace msddk { ;

class CKeTime {

public:
	CKeTime()
	{
		memset(&GelinTime, 0, sizeof(GelinTime));
		memset(&LocalTime, 0, sizeof(LocalTime));
		memset(&NowFields, 0, sizeof(NowFields));

		KeQuerySystemTime(&GelinTime);
		ExSystemTimeToLocalTime(&GelinTime, &LocalTime);
		RtlTimeToTimeFields(&LocalTime, &NowFields);
	}
	CKeStringA GetCurrentTimeString()
	{
		CKeStringA strTime;
		strTime.Format("%4d-%02d-%02d %02d:%02d:%02d",
			NowFields.Year,
			NowFields.Month,
			NowFields.Day,
			NowFields.Hour,
			NowFields.Minute,
			NowFields.Second);
		return strTime;
	}

private:
	LARGE_INTEGER GelinTime;
	LARGE_INTEGER LocalTime;
	TIME_FIELDS NowFields;
};

};