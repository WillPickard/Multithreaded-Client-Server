//routines.h
//in this file are defined worker routines for threads to execute

#ifndef SERVER_ROUTINES_H
#define SERVER_ROUTINES_H
//class Server;
//class ConcurrentDelegator;
#include "../Headers/worker.h"
#include "../Headers/server.h"
/**
#include "../Headers/client.h"
#include "../Headers/concurrent_delegator.h"

/**/
#include <netinet/in.h>






/***
* Create a thread to do delegate an accepted connection
*
* The class below is used to pack this all up for the delegateRoutine
***/
class DelegateBox {
	public:
		int socketFd;
		sockaddr * remoteAddr;
		Server * server;
		DelegateBox(int socketFd, sockaddr * remoteAddr, Server * server):
			socketFd(socketFd),
			remoteAddr(remoteAddr),
			server(server)
			{};
		int getSocketFd() const
		{
			return socketFd;
		}	
		sockaddr * getRemoteAddr() const 
		{
			return remoteAddr;
		}
		Server * getServer() const
		{
			return server;
		}
};

/****
*
*	Handle a new socket connection
*
*****/
void * delegateRoutine(void *b)
{
	//cast it out of the void pointer 
	DelegateBox * box = (DelegateBox *) b;
	//into a worker pointer
	box->getServer()->getDelegator()->handle(box->getSocketFd(), box->getRemoteAddr());
	//exit without error
	pthread_exit(0);
}


#endif