//sender.h
#ifndef SENDER_H
#define SENDER_H

#include <exception>

#include "worker.h"

#ifndef SEND_EXCEPTION
#define SEND_EXCEPTION
class SendException : public std::exception
{
	private:
	public:
		SendException(){};
		
		virtual const char * what() const throw()
		{
			return "Error sending data";
		}
};
#endif

class SentLessThan0Exception : public std::exception
{
	private:
	public:
		SentLessThan0Exception(){};
		
		virtual const char * what() const throw()
		{
			return "Sent less than 0 bytes";
		}
};


class Sender : public Worker
{
	private:

	public:
		//the virtual method inherited from worker
		Sender(const char * command, int socket, struct sockaddr * remoteAddr, Node * n):
			Worker(command, socket, remoteAddr, n)
			{};

		Sender(int socket, struct sockaddr * remoteAddr, Node * n):
			Worker(socket, remoteAddr, n)
			{};

		void doWork();

		int sendFile(const char * filename);
		int transmit(const char *);

		void sendFileInfo(std::string, int);

		int terminateCommand(int);
		int terminateCommand(const char *);
	//	void waitForFileSizeRequest(int);

};
#endif