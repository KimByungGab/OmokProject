#pragma once

#include <Windows.h>
#include <iostream>
#include <locale>

using namespace std;

class StringConvert
{
public:
	void w2s(wstring& wstr);
	void s2w(string& str);
};

