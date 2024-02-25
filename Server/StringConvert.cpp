#include "StringConvert.h"

void StringConvert::w2s(wstring& wstr)
{
	locale loc("");

	auto& facet = use_facet<codecvt<WCHAR, char, mbstate_t>>(loc);
	wstring_convert<remove_reference<decltype(facet)>::type, WCHAR>(&facet).to_bytes(wstr);
}

void StringConvert::s2w(string& str)
{
	locale loc("");

	auto& facet = use_facet<codecvt<WCHAR, char, mbstate_t>>(loc);
	wstring_convert<remove_reference<decltype(facet)>::type, WCHAR>(&facet).from_bytes(str);
}
