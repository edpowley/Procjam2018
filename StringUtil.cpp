#include "stdafx.h"
#include "StringUtil.h"

std::string trimWhiteSpace(const std::string& str)
{
	const char* whiteSpaceChars = " \t\r\n";
	auto firstPos = str.find_first_not_of(whiteSpaceChars);
	auto lastPos = str.find_last_not_of(whiteSpaceChars);

	if (firstPos == std::string::npos)
		return "";
	else
		return str.substr(firstPos, lastPos - firstPos + 1);
}
