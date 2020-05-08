#include <stdio.h>
#include <windows.h>
#include "winlock.h"
#include "mistl.h"


struct Test_t
{
	char a;
	int  b;
	char * p;
	Test_t()
	{
		printf("test_t construct\n");
	}
	Test_t(const Test_t & t)
	{
		a = 0; b = 1; p = 0;
		printf("test_t copy construct\n");
	}
};

struct Test2_t
{
	char k[3];
};

bool operator == (const Test2_t & t1, const Test2_t & t2)
{
	return memcmp(&t1, &t2, sizeof(t1)) == 0;
}

void test_string()
{
	mistd::mistring  str, str2;
	str.assign("123");
	str.append("456");
	str.swap(str2);
	str.insert(0, "abc");
	str.insert(2, "bb");
	str2.replace(0, 2, "abc");
	str2.replace(0, 1, "a");
	str2.swap(str2);

	if("abc" == str2) printf("ooo\n");
	if(str2 == "abc") printf("ooo2\n");

	str = str2;
	str.resize(10);
	str.append("223");
	str += "aaa"; str += str2;
	memcpy(&str[0], "bbbbbbbbbbb", 10);
	for(mistd::mistring::iterator it = str.begin(); it != str.end(); it++)
	{
		printf("%c", *it ? *it : '?');
	}

	size_t pos  = str.find("23");
	size_t pos2 = str.find('a');
	mistd::miwstring str3(L"aaa"), str4;
	str3 += L"bbb";

	const wchar_t * p = str4.c_str();
	size_t pos3 = str3.find(L"ab");

	mistd::miwstring str5 = L"test+", str6;
	str6 = str5 + L"a" + L"123";


	for(mistd::mistring::iterator it = str2.begin(), end = str2.end(); it != end; it++)
	{
		printf("%c", *it);
	}

	printf(" = str2\n");
	printf("%d:%s, %d:%s\n", str.length(), str.c_str(), str2.length(), str2.c_str());
}

void test_list()
{
	mistd::milist<Test_t> lstTest1;
	Test_t test1;
	lstTest1.push_back(test1);

	mistd::milist<int> lt, lt2;
	int    v[3] = {1, 2, 3};
	lt.push_back(v[0]);
	lt.push_back(v[1]);
	lt.push_back(v[2]);
	lt.push_back(4);

	lt2      = lt;
	int   t  = lt.back();

	int * t1 = &lt.back();
	//mistd::milist<int>::milist_node_t * node = mistd::milist_data_to_node(t1);

	const mistd::milist<int> & plst = lt;
	bool emp = plst.empty();
	plst.back();
	lt.back();

	lt.pop_back();
	lt.pop_front();
	for(mistd::milist<int>::const_iterator it = lt.begin(), end = lt.end(); it != end; it++)
	{
		printf("%d,", *it);
	}
	printf("\n");


	::mistd::milist<int> lt3(lt2);
	mistd::milist<mistd::mistring> lstS, lstS2;
	mistd::mistring str1 = "124";
	lstS.push_back(str1);
	lstS.push_back("addf");
	lstS.push_back(mistd::mistring("123"));
	lstS.push_back(mistd::mistring("234"));
	lstS.push_back(mistd::mistring("345"));

	for(mistd::milist<mistd::mistring>::iterator it = lstS.begin(), end = lstS.end(); it != end; it++)
	{
		printf("%s,", it->c_str());
		//if(strcmp(it->c_str(), "234") == 0) { lstS.erase(it); break; }
	}
	printf("\n");

	lstS2.push_back(mistd::mistring("aaa"));
	lstS2.push_back(mistd::mistring("bbb"));
	lstS2.push_back(mistd::mistring("ccc"));

	mistd::milist<mistd::mistring>::const_iterator it2 = lstS2.begin(), it3, it4;
	it3 = lstS.begin();
	it4 = it3; it4++; it4++; it4++; it4++;
	lstS2.splice(it2, lstS, it3, it4);
	lstS2.splice(lstS2.begin(), lstS);
	for(mistd::milist<mistd::mistring>::const_iterator it = lstS2.begin(), end = lstS2.end(); it != end; it++)
	{
		printf("%s,", it->c_str());
	}
	printf("\n");

	mistd::DumpMemwPool();
}

void test_vector()
{
	mistd::mivector<int> vec1, vec2;
	vec1.push_back(1);
	vec1.push_back(2);
	vec1.push_back(3);
	vec1.push_back(4);
	vec1.push_back(5);
	vec1.pop_back();

	vec2 = vec1;

	mistd::mivector<int>::iterator it2 = vec2.begin(); it2++; it2++;
	//vec2.erase(vec2.begin());
	printf("%d\n", *it2); vec2.erase(it2);
	mistd::mivector<int>::iterator it3 = vec2.begin(); it3++;
	vec2.insert(it3, 0);

	for(mistd::mivector<int>::iterator it = vec2.begin(); it != vec2.end(); it++) printf("%d,", *it);
	printf("\n");
	for(int i = 0; i < vec1.size(); i++) printf("%d,", vec1[i]);
	printf("\n");


	mistd::mivector<unsigned int> vec3(3);
	vec3.push_back(1);
	for(mistd::mivector<unsigned int>::iterator it = vec3.begin(); it != vec3.end(); it++)
	{
		printf("%d,", *it);
	}
	vec3.pop_back();

	mistd::mivector<Test_t> vec4;
	Test_t t;  t.a = 1; t.b = 2; t.p = "123";
	vec4.push_back(t); t.a++;
	vec4.push_back(t); t.b++;
	vec4.push_back(t);
	vec4.pop_back();

	mistd::mivector<mistd::mistring> vecS1;
	vecS1.push_back(mistd::mistring("111"));
	vecS1.push_back("222");
	vecS1.pop_back();

	mistd::mivector<char> vecS2;
	mistd::mistring str1("abcdefgh1234567890");
	for(int i = 0; i < str1.length(); i++) vecS2.push_back(str1.at(i));
	vecS2.push_back(0);

	return;
}

void test_map()
{
	int r = 0;
	mistd::mimap<int, int> map1, map12;
	r = map1.insert(mistd::mimake_pair(1, 2));  assert(!r);
	r = map1.insert(mistd::mimake_pair(1, 3));  assert(r);
	r = map1.insert(mistd::mimake_pair(3, 5));  assert(!r);

	mistd::mimap<int, int>::iterator it1;
	it1 = map1.find(1);

	bool bempty = map1.empty();

	map12   = map1;
	int v1  = map1[1];
	int v11 = map1[2];

	map1[3] = 4;

	for(mistd::mimap<int, int>::const_iterator it = map1.begin(); it != map1.end(); it++)
	{
		printf("%d -->%d\n", it->first, it->second);
	}

	map1.erase(1);

	mistd::mimap<Test2_t, int> map2;
	Test2_t t2 = { "00" }, t3 = { "00" };
	r = map2.insert(mistd::mimake_pair(t2, 3)); assert(!r);
	r = map2.insert(mistd::mimake_pair(t3, 2)); assert(r);

	for(mistd::mimap<Test2_t, int>::iterator it = map2.begin(); it != map2.end(); it++)
	{
		printf("%d -->%d\n", it->first.k[0], it->second);
	}

	mistd::mimap<mistd::mistring, int> map3, map32;
	mistd::mistring str1("123"), str2("123");
	r = map3.insert(mistd::mimake_pair(str1, 1)); assert(!r);
	r = map3.insert(mistd::mimake_pair(str2, 3)); assert(r);
	r = map3.insert(mistd::mimake_pair("000", 3)); assert(!r);

	mistd::mimap<mistd::mistring, int>::iterator it3, it32;
	it3  = map3.find("000"); assert(it3  != map3.end());
	it32 = map3.find("111"); assert(it32 == map3.end());

	map32 = map3;
	r = map32.insert(mistd::mimake_pair("222", 4)); assert(!r);
	map3.erase(it3);

	map3["adb"] = 10;
	for(mistd::mimap<mistd::mistring, int>::const_iterator it = map3.begin(); it != map3.end(); it++)
	{
		printf("%s -->%d\n", it->first.c_str(), it->second);
	}

	mistd::mimap<mistd::mistring, int> map4;
	map4.swap(map3);

	return;
}

void test_map2()
{
	mistd::mimap<int, mistd::mimap<int, int> > mx1;
	mistd::mimap<int, int> m1;

	//m1[0] = 0;
	//m1[1] = 1;
	m1[2] = 2;

	mx1[10] = m1;

	MILOG("_________________________\n");
	return;
}

void test_set()
{
	int r = 0;
	mistd::miset<int> set1;
	set1.insert(5);
	set1.insert(1);
	set1.insert(3);
	set1.insert(4);
	set1.insert(2);
	set1.insert(6);
	set1.insert(0);

	for(mistd::miset<int>::const_iterator it = set1.begin(); it != set1.end(); it++)
	{
		printf("%d,", *it);
	}
	printf("\n");

	mistd::miset<int>::iterator it1;
	r = set1.insert(2); assert(r);
	it1 = set1.find(3); assert(it1 != set1.end());
	printf("find ok -->%d\n", *it1);
	set1.erase(it1);

	for(mistd::miset<int>::const_iterator it = set1.begin(); it != set1.end(); it++)
	{
		printf("%d,", *it);
	}
	printf("\n");
	return;
}

void test_pair()
{
	//mistd::mipair<mistd::mistring, int> p1;
	mistd::milist<mistd::mipair<mistd::mistring, int> > lt1;

	lt1.push_back(mistd::mimake_pair("124", 2));

	return;
}
