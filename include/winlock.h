#pragma once


class IGenerateLocker 
{
public:
	virtual void Lock()   = 0;
	virtual void Unlock() = 0;
};


class CAutoLocker
{
public:
	IGenerateLocker * m_l;
	CAutoLocker(IGenerateLocker * l) : m_l(l) { m_l->Lock(); }
	~CAutoLocker() { m_l->Unlock(); }
};


class CCriticalSection : public IGenerateLocker
{
public:
	CRITICAL_SECTION  m_cs;
	CCriticalSection()  { InitializeCriticalSection(&m_cs); }
	~CCriticalSection() { DeleteCriticalSection(&m_cs); }
	void Lock()         { EnterCriticalSection(&m_cs); }
	void Unlock()       { LeaveCriticalSection(&m_cs); }
	static CCriticalSection * GetInstance() 
	{ 
		static CCriticalSection g_cs; return &g_cs;
	}
};


// thread safe.
#define __mimalloc_free__

namespace mistd 
{
extern void * MwAlloc(unsigned int size);
extern void   MwFree(void * );
}

inline void * mimalloc(size_t sz)
{
	CAutoLocker l(CCriticalSection::GetInstance());
	return mistd::MwAlloc(sz);
}

inline void  mifree(void * p)
{
	CAutoLocker l(CCriticalSection::GetInstance());
	mistd::MwFree(p);
}
