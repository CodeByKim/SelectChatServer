#include "Util.h"

void SplitString(std::string* source, std::vector<std::string>* outSplits)
{
	std::istringstream iss(*source);
	std::string splitString;
	while (std::getline(iss, splitString, ' '))
	{
		outSplits->push_back(splitString);
	}
}

void ToUpperString(std::string* dest, std::string::iterator begin, std::string::iterator end, int length)
{
	dest->resize(length);

	std::transform(begin,
		           end,
			       dest->begin(),
		           std::toupper);
}
