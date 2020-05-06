#pragma once


#include <memory.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>


// 修改默认的内存池参数.
#ifndef  MWP_BLOCK_MIN_SIZE
#define  MWP_BLOCK_MIN_SIZE              (1024*4)
#endif

#define  DESCLARE_NAMESPACE_MISTD_BEGIN  namespace mistd {
#define  DESCLARE_NAMESPACE_MISTD_END    }

#ifndef  offsetof
#define  offsetof(_type, _member)        ((int)&((_type *)0)->_member)
#endif


DESCLARE_NAMESPACE_MISTD_BEGIN
#define  __mpmalloc      malloc
#define  __mpfree        free
#include "mimalloc.h"


#define mitchar          char
#define misize_t         size_t
#define miuchar          unsigned char
#define miuint           unsigned int
#define miint64          __int64

#define miassert         assert
#define mimalloc         MwAlloc
#define mifree           MwFree
#define mimemset         memset
#define mimemcpy         memcpy
#define mimemmove        memmove


template <typename T>
struct MoveNext { T * t; MoveNext(T * n) : t(n ? n->next : 0) {} };

template <typename T>
struct MovePrev { T * t; MovePrev(T * n) : t(n ? n->prev : 0) {} };

template <typename T>
struct MoveAdd  { T * t; MoveAdd (T * n) : t(n ? (n + 1) : 0) {} };

template <typename T>
struct MoveDec  { T * t; MoveDec (T * n) : t(n ? (n - 1) : 0) {} };


template <typename TNode, typename TData, typename TMove>
class GenericIterator
{
public:
	typedef GenericIterator<TNode, TData, TMove> GIterator;
	GenericIterator(TNode * it = 0) : m_node(it),  m_index(0) {}
	GenericIterator(TNode & it)     : m_node(&it), m_index(0) {}

	bool        operator != (const GIterator & gi) const { return m_node != gi.m_node; }
	bool        operator == (const GIterator & gi) const { return m_node == gi.m_node; }
	GIterator   operator ++ ()    const { GIterator * p = (GIterator*)this; p->next(); return *this; }
	GIterator   operator ++ (int) const { GIterator * p = (GIterator*)this; TNode *n = m_node; p->next(); return n; }

	TData     & operator *  ()    const { return      m_node->data; }
	TData     & operator *  ()          { return      m_node->data; }
	TData     * operator -> ()    const { return     &m_node->data; }
	TData     * operator -> ()          { return     &m_node->data; }
	TNode     * get() const             { return      m_node;  }
	int         index() const           { return      m_index; }
	void        next()                  { if(m_node){ TMove m(m_node); m_node = m.t; m_index++; }}


private:
	TNode * m_node;
	int     m_index;
};

template <typename TData, typename TMove>
class GenericIterator<TData, TData, TMove>
{
public:
	typedef GenericIterator<TData, TData, TMove> GIterator;
	GenericIterator(TData * it = 0) : m_node(it),  m_index(0) {}
	GenericIterator(TData & it)     : m_node(&it), m_index(0) {}

	bool        operator != (const GIterator & gi) const { return m_node != gi.m_node; }
	bool        operator == (const GIterator & gi) const { return m_node == gi.m_node; }
	GIterator   operator ++ ()    const { GIterator * p = (GIterator*)this; p->next(); return *this; }
	GIterator   operator ++ (int) const { GIterator * p = (GIterator*)this; TData *n = m_node; p->next(); return n; }

	TData     & operator *  ()    const { return     *m_node;  }
	TData     & operator *  ()          { return     *m_node;  }
	TData     * operator -> ()    const { return      m_node;  }
	TData     * operator -> ()          { return      m_node;  }
	TData     * get() const             { return      m_node;  }
	int         index() const           { return      m_index; }
	void        next()                  { if(m_node){ TMove m(m_node); m_node = m.t; m_index++; }}


private:
	TData * m_node;
	int     m_index;
};


extern int    micalc_hash_string(miuchar * str, int len);
struct miscalar_ptr {};
struct minonscalar_ptr {};

template <typename T> 
struct miptr_type_helper  { typedef minonscalar_ptr miptr_type; };

template <typename T> 
void midestroy_node_node(T * p, minonscalar_ptr) { if(p) p->~T(); }

template <typename T> 
void midestroy_node_node(T * p, miscalar_ptr)    {}

template <typename T>
void midestroy_node(T * p) { typename miptr_type_helper<T>::miptr_type t; midestroy_node_node(p, t); }

template <typename T> 
int  micalc_hash_node(const T & n, miscalar_ptr)    { return n; }

template <typename T> 
int  micalc_hash_node(const T & n, minonscalar_ptr) { return micalc_hash_string((miuchar*)&n, (int)sizeof(n)); }

#define MISCALAR_TYPE_FUNCTOR(_type) \
	template <> struct miptr_type_helper<_type> { typedef miscalar_ptr miptr_type;};

#define DESCLARE_MISCALAR_TYPE(_V) \
	_V(int) \
	_V(char) \
	_V(miint64) \

DESCLARE_MISCALAR_TYPE(MISCALAR_TYPE_FUNCTOR)


template <typename T1, typename T2>
struct mipair
{
	T1  first;
	T2  second;

	template <typename TT1, typename TT2>
	mipair(const TT1 & t1, const TT2 & t2) : first(t1), second(t2) {}
	~mipair() {}

	template <typename TT1, typename TT2>
	mipair(const mipair<TT1, TT2> & node) : first(node.first), second(node.second) {}

	template <typename TT1, typename TT2>
	mipair & operator = (const mipair<TT1, TT2> & node) { first = node.first; second = node.second; }
};

template <typename T1, typename T2>
typename mipair<T1, T2> mimake_pair(const T1 & t1, const T2 & t2)
{
	mipair<T1, T2> mp(t1, t2);
	return mp;
}

template <typename T1, typename T2>
typename mipair<T1*, T2> mimake_pair(const T1 * t1, const T2 & t2)
{
	mipair<T1*, T2> mp((T1*)t1, t2);
	return mp;
}

template <typename T1, typename T2>
typename mipair<T1*, T2*> mimake_pair(const T1 * t1, const T2 * t2)
{
	mipair<T1*, T2*> mp((T1*)t1, (T2*)t2);
	return mp;
}

template <typename T1, typename T2>
typename mipair<T1, T2*> mimake_pair(const T1 & t1, const T2 * t2)
{
	mipair<T1, T2*> mp(t1, (T2*)t2);
	return mp;
}


DESCLARE_NAMESPACE_MISTD_END


// for mialloc.
#include <stdio.h>
#include <time.h>
extern  FILE * g_milog_file;
//#define MILOG(_f, ...)          { fprintf(g_milog_file, "[%04d] " _f, _time32(0), __VA_ARGS__); fflush(g_milog_file); }
#define MILOG(_f, ...)          { fprintf(g_milog_file, _f, __VA_ARGS__); fflush(g_milog_file); }
#define MIMEMORY_AUTO_DUMP()    mistd::DumpMemwPool();

#ifdef _DEBUG
#define MILOG_DEBUG(_f, ...)    MILOG(_f, __VA_ARGS__)
#define RUN_DEBUG(_f)           //_f
#else
#define MILOG_DEBUG      
#define RUN_DEBUG(_f)    
#endif


// __PLACEMENT_NEW_INLINE --> vc\include\new.h
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void * operator new (misize_t sz, void * p)
{
	return p;
}

inline void   operator delete(void * p, void *)
{
}
#endif
