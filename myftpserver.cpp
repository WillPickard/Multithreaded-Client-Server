#include "./Classes/server.cpp"
#include "./Classes/server_runner.cpp"
#include "./Classes/server_terminator.cpp"

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

using namespace std;

int getPortNumber(const char *);

int main(int argc, const char * argv[]){
	if(argc != 3)
	{
		printf("Invalid call. Proper syntax is myftpserver [portnumber] [terminatenumber], where portnumber is the portnumber to listen on and terminatenumber is the port to listen for terminate calls on.\n");
		exit(1);
	}

	int portNumber = getPortNumber(argv[1]);
	int terminateNumber = getPortNumber(argv[2]);

	printf("myftpserver is starting up...\n");

	//create the server by passing in 
	//the port number, the socket
	//domain, the socket type,
	//and the socket protocol
	//0 means "figure it out"
	Server * server = new Server(portNumber, AF_INET, SOCK_STREAM, 0);
	Server * terminateServer = new ServerTerminator(terminateNumber, AF_INET, SOCK_STREAM, 0, server);

	//server->run();
	ServerRunner * runner = new ServerRunner(server);
	ServerRunner * terminator = new ServerRunner(terminateServer);

	pthread_t runnerId, terminatorId;

	runnerId = server->getThreadController()->newThread(runner);
	terminatorId = server->getThreadController()->newThread(terminator);

	server->getThreadController()->join(runnerId);
	server->getThreadController()->join(terminatorId);

	printf("\nmyftpserver dropping off main...\n");
};

//extract the port number from the command line input
int getPortNumber(const char *c)
{
	int number;
	std::string message = "The port number must be an integer greater than 0";
	istringstream ss(c);
	if(!(ss >> number))
	{
		cout << message << endl;
		exit(1);
	}
	if(number <= 0)
	{
		cout << message << endl;
		exit(1);
	}

	return number;

}
