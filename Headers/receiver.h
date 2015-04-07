//receiver.h
#ifndef RECEIVER_H
#define RECEIVER_H

#include "worker.h"
#include <exception>

//define the exception to throw when we receive less than 0 bytes via a call to recv()
class BytesLessThan0Exception : public std::exception
{
	private:
	public:
		BytesLessThan0Exception()
			{};

		
		virtual const char * what() const throw()
		{
			return "Received less than 0 bytes";
		}
};

class Receiver : public Worker 
{
	private:
	public:
		Receiver(const char * command, int socket, struct sockaddr * remoteAddr, Node * n):
			Worker(command, socket, remoteAddr, n)
			{};

		Receiver(int socket, struct sockaddr * remoteAddr, Node * n):
			Worker(socket, remoteAddr, n)
			{};

		void doWork();

		//int recvFile(const char * filename);
		int recvFile();
		int receive(char *, int);

		FileInfo * requestFileInfo();
};
#endif
