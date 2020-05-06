#pragma once


#include "micomm.h"


DESCLARE_NAMESPACE_MISTD_BEGIN
#include "mistring.h"
#include "milist.h"
#include "mivector.h"
#include "mimap.h"
#include "miset.h"
DESCLARE_NAMESPACE_MISTD_END


// 
// 使用说明
//#include "mistl.h"
// 
// 下面用来替换stl
#ifdef  __USE_MISTL_REPLACE_STL__
#define  std        mistd
#define  pair       mipair
#define  make_pair  mimake_pair
#define  vector     mivector
#define  string     mistring
#define  wstring    miwstring
#define  list       milist
#define  map        mimap
#define  set        miset
#endif


#ifdef __ATLSTR_H__
DESCLARE_NAMESPACE_MISTD_BEGIN
template <> inline int micalc_hash_node<CString>(const CString & s, minonscalar_ptr)
{
	CString * ps = (CString*)&s;
	return micalc_hash_string((miuchar*)ps->GetBuffer(), (int)s.GetLength()*sizeof(TCHAR));
}
DESCLARE_NAMESPACE_MISTD_END
#endif
