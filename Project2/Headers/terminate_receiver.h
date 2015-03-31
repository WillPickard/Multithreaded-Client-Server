//terminate_receiver.h
#ifndef TERMINATE_RECEIVER_H
#define TERMINATE_RECEIVER_H

#include "receiver.h"
#include "server_terminator.h"

class TerminateReceiver : public Receiver
{

	public:
		TerminateReceiver(const char * command, int socket, struct sockaddr * remoteAddr, Node * n):
			Receiver(command, socket, remoteAddr, n)
			{};

		TerminateReceiver(int socket, struct sockaddr * remoteAddr, Node * n):
			Receiver(socket, remoteAddr, n)
			{};

		void doWork();

		void terminateCommand(int);
		
};

#endif