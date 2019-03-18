#pragma once
#include <kutil/sentry.h>
#include "jansson.h"

struct json_sentry
{
	static void* default_value() { return 0; }
	template<class _Ptr> static bool equal_to(_Ptr l, _Ptr r) { return l == r; }
	template<class _Ptr> static void destroy(_Ptr p) { if (p)json_decref(p); }
};

typedef   ke_sentry<json_t*, json_sentry> json_ptr;