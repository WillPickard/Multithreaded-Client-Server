//client_cl_reader.h
//extends the sender class
//basically just read input from the command line and send it

#ifndef CLIENT_CL_READER 
#define CLIENT_CL_READER

static const char * PROMPT_TEXT = "myftp>";

#include <iostream> //std::cin.getline

#include "sender.h"

class ClientCLReader : public Sender
{
	private:
	public:
		ClientCLReader(const char * command, int socket, struct sockaddr * remoteAddr, Node * n):
			Sender(command, socket, remoteAddr, n)
			{};

		ClientCLReader(int socket, struct sockaddr * remoteAddr, Node * n):
			Sender(socket, remoteAddr, n)
			{};
		//override inherited method
		void doWork();

		//show prompt asking for input
		void prompt(char *, int);


};

#endif