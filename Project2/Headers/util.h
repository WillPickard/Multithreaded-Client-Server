//util.h
#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <array>
#include <string>
#include <string.h>
#include <sstream>
#include <stdio.h>

class Util 
{
public:
	Util(){};

	static void intToString(int, char *);

	static double parseFloat(const std::string);
	static double parseFloat(const char *);

	static int parseInt(const std::string);
	static int parseInt(const char *);
	
	static std::vector<std::string> split(char, const std::string&);
	static std::vector<std::string> split(char, const char *);
	static std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems);
};
#endif