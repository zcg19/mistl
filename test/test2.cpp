#include "mistl.h"
#include <atlstr.h>


void test_string();
void test_list();
void test_vector();
void test_map();
void test_set();
void test_map2();
void test_pair();



struct Test1
{
	Test1(int b) { a = 1; };
	int a;
};

struct Test2
{
	Test2(int b) { a = 2; };
	int a;
};

template <typename T1>
int test_make_pair(int a)
{
	return T1(a).a;
}



void main()
{
	test_pair();

	test_map2();
	MIMEMORY_AUTO_DUMP();

	{
		mistd::mimap<CString, int> gg;
		gg[L"000"] = 0;
	}
	//mistd::DumpMemwPool();

	test_string();
	//MIMEMORY_AUTO_DUMP();

	test_vector();
	//mistd::DumpMemwPool();
	test_map();
	//mistd::DumpMemwPool();

	
	test_list();
	mistd::DumpMemwPool();
	test_set();
	mistd::DumpMemwPool();

	return;
}
