#include "Classes/client.cpp"

#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

const std::string errorMessage = "Invalid call. Valid is myftp [server name] [server port number] [server termiante port number]. Terminating.";

int extractServerPort(const char *);
const char * extractServerName(const char *);
void error();

int main(int argc, const char * argv[])
{
	if(argc != 4)
	{
		error();
	}

	int serverPort = extractServerPort(argv[2]);
	const char * serverName = extractServerName(argv[1]);
	int tpn = extractServerPort(argv[3]);

	Client * client = new Client(serverPort, AF_INET, SOCK_STREAM, 0, serverName, tpn);
	client->run();
}

void error()
{
	cout << errorMessage << endl;
	exit(1);
}

int extractServerPort(const char *c)
{
	
	int number;
	istringstream ss(c);
	if(!(ss >> number))
	{
		error();
	}
	if(number <= 0)
	{
		error();
	}

	return number;
}

const char * extractServerName(const char *c)
{
	return c;
}
