//util.cpp
#include "../Headers/util.h"


//put i into the buf with size size
void Util::intToString(int i, char * buf)
{
	sprintf(buf, "%d", i);
}

//parse a double out of string
double Util::parseFloat(const std::string s)
{
	return parseFloat(s.c_str());
}
double Util::parseFloat(const char * s)
{
	double number;
	std::istringstream ss(s);
	
	ss >> number;

	return number;
}

//parse int out of string
int Util::parseInt(const std::string s)
{
	return parseInt(s.c_str());
}
int Util::parseInt(const char * s)
{
	int number;
	std::istringstream ss(s);
	
	ss >> number;

	return number;
}

//split the string on the delimeter and return a vector where each index is a chunk of the search
//std::vector<const char *> Util::split(char delim, const std::string s)
//{
//	return split(delim, s.c_str());
//}
std::vector<std::string> Util::split(char delim, const char * search)
{
/*//	printf("\t\tUtil::split() ... delim: %c, search: %s\n", delim, search);
	std::vector<const char *> ret;
	//std::vector<char> parsed;

	int len = strlen(search);
	//int last = 0;

	std::string current = "";
	for(int i = 0; i < len; i++)
	{
		char c = search[i];
	//	printf("\t\t\tUtil::split() ... c: %c\n", c);
		if(c == delim)
		{
			printf("\t\t\tUtil::split() ... push_back: %s\n", current.c_str());	

			ret.push_back(current.c_str());
			current.erase(current.begin(),current.end());
			printf("\t\t\tUtil::split() ... back: %s\n", ret.back());
		}
		else
		{
			current += c;
			//parsed.push_back(c);
		}
	}

	current[current.length()] = '\0';
	const char * last = current.c_str();
	//printf("\t\t\tUtil::split() ... pushing back: %s, len: %d\n", last, strlen(last));
	ret.push_back(last);
//	printf("\t\t\tUtil::split() ... size of return vecotr: %d\n", ret.size());
	return ret;
	*/
	std::string s (search);
	return Util::split(delim, s);
}
std::vector<std::string> Util::split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> Util::split(char delim, const std::string &s) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}