//routines.h
//in this file are defined worker routines for threads to execute

#ifndef ROUTINES_H
#define ROUTINES_H

//class ConcurrentDelegator;
#include "../Headers/worker.h"
/**
#include "../Headers/client.h"
#include "../Headers/concurrent_delegator.h"

/**/
#include "../Headers/server.h"
#include <netinet/in.h>




/***************
*
*	This class will encapsulate 
*	the worker class
*
*****************/
class WorkerBox {
	public: 
		 Worker * worker;

		 WorkerBox(Worker * worker):
		 	worker(worker)
		 	{};

		 Worker * getWorker() const
			{
				return worker;
			}	
};

/***
*
*	Worker routine is the routine the thread will execute
*	The void pointer is an object of type WorkerBox
*	To retreive the worker from the box just cast
*	it to a WorkerBox * then call ->getWorker()
*
****/
void * workerRoutine(void *b)
{
	//cast it out of the void pointer 
	WorkerBox * box = (WorkerBox *) b;
	Worker * worker = box->getWorker();

	char buf[256];
	int bytesReceived  = 0;
	//we have to wait for the command 
	do
	{
		//memset it out because the internet says to
		memset(buf, 0, sizeof(buf));
		bytesReceived = recv(worker->getFileDescriptor(), buf, sizeof(buf), 0);

		worker->setCommand((const char *) buf);
		worker->execute();

		if(bytesReceived <= 0)
		{
			//handleError("recv");
			//printf("\tbytesreceived <= 0...\n");
			//handleError will exit but whatevs
			//pthread_exit(0);
		}

		//printf("\tReceived %s... creating worker...\n", buf);
	} while(!worker->isDone());
	//exit without error
	//printf("Worker is done in its workerRoutine()\n");
	pthread_exit(0);
}


/***
*
*	Transmission routine and encapsulation
*
*	Transmissions occur when we want to send a file or something
**
class ClientBox {
	public:
		Client * client;

		ClientBox(Client * c):
			client(c)
			{};

		Client * getClient() const 
		{
			return client;
		}
};


void * transmitRoutine(void * cb)
{
	ClientBox * box = (ClientBox *)cb;
	Client * client = box->getClient(); 
	char buf[256];

	while(printf("myftp> "), std::cin.getline(buf, sizeof(buf)))
		{
			//buf[sizeof(buf)] = "\0";			
			if(send(client->getSocket(), buf, sizeof(buf), 0) == -1)
			{
				//handleError("send");
			}

			//totalBytesSent += sizeof(buf);
		}
}
*/

/***
*
*	Receive routine 
*
***
void * receiveRoutine(void *cb)
{
	ClientBox * box = (ClientBox *)cb;
	Client client = box->getClient(); 
}
/**/



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