//client_receiver.h

#ifndef CLIENT_RECEIVER_H
#define CLIENT_RECEIVER_H

#include "receiver.h"

class ClientReceiver : public Receiver
{
	private:
	public:
		ClientReceiver(const char * command, int socket, struct sockaddr * remoteAddr, Node * n):
			Receiver(command, socket, remoteAddr, n)
			{};

		ClientReceiver(int socket, struct sockaddr * remoteAddr, Node * n):
			Receiver(socket, remoteAddr, n)
			{};
		//override inherited method
		void doWork();

		int receive(char *, int);
};

#endif