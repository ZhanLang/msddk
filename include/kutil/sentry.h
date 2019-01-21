#pragma once
namespace msddk { ;
struct ke_default_sentry
{
	static void* default_value() { return 0; }
	template<class _Ptr> static bool equal_to(_Ptr l, _Ptr r) { return l == r; }
	template<class _Ptr> static void destroy(_Ptr p) { delete p; }
};

struct ke_default_array_sentry
{
	static void* default_value() { return 0; }
	template<class _Ptr> static bool equal_to(_Ptr l, _Ptr r) { return l == r; }
	template<class _Ptr> static void destroy(_Ptr p) { delete[] p; }
};



struct ke_handle_sentry
{
	static void* default_value() { return 0; }
	template<class _Ptr> static bool equal_to(_Ptr l, _Ptr r) { return l == r; }
	template<class _Ptr> static void destroy(_Ptr p) { if (p)ZwClose(p); }
};

template<class _Ptr,
	class _Traits = ke_default_sentry>
	struct ke_sentry
{
public:
	ke_sentry(const _Traits& tr = _Traits()) : m_tr(tr) { m_p = (_Ptr)m_tr.default_value(); }
	ke_sentry(_Ptr p, const _Traits& tr = _Traits()) : m_p(p), m_tr(tr) {}
	~ke_sentry() { m_tr.destroy(m_p); }
	ke_sentry& operator = (_Ptr p) { if (!m_tr.equal_to(m_p, p)) { m_tr.destroy(m_p); m_p = p; } return *this; }
	_Ptr detach() { _Ptr tmp = m_p; m_p = (_Ptr)m_tr.default_value(); return tmp; }
	operator _Ptr () const { return m_p; }
	_Ptr operator -> () const { return m_p; }
	void reset() { m_p = (_Ptr)m_tr.default_value(); }
	//	_Ptr* operator & () { return &m_p; }
public:
	_Ptr m_p;
	_Traits m_tr;
private:
	ke_sentry(const ke_sentry&);
	ke_sentry& operator = (const ke_sentry&);
};
};
