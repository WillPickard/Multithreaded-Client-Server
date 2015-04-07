#ifndef CLIENT_H
#define CLIENT_H

static const int CLIENT_RECEIVER_BUFFER_SIZE = 512; //max amount in bytes the clientREceiverRoutine can handle at a time
static const int CLIENT_SENDER_BUFFER_SIZE = 512; //max amount in bytes the clientSenderRoutine can handle at a time

#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <netinet/in.h>


//#include "thread_controller.h"

//class ConcurrentDelegator;
//class Semaphore;
//#include "worker.h"
//#include "../Headers/concurrent_delegator.h"
#include "semaphore.h"
#include "node.h"
#include "client_cl_reader.h"
#include "client_receiver.h"
#include "receiver.h"
#include <queue>

//#include "../Routines/routines.h"


class Client : public Node {
	private:
		struct addrinfo * serverAddress;
		struct addrinfo * terminateAddress;

		Semaphore * outputMutex;
		std::queue<char *> * outputQueue;

		const char * serverName;

		const char * currentCommand;

		//std::unordered_map<int, struct CommandStatus *> commands;

		ClientCLReader * sender;
		ClientReceiver * receiver;


	public:
		void run();

		//constructor with no server name
		Client(int, int, int, int);
		//constructor with a server name as final param
		Client(int, int, int, int, const char *);
		//constructor with servername and terminate port number
		Client(int, int, int, int, const char *, int);
		
		struct addrinfo * getServerAddress() const;
		struct addrinfo * getTerminateAddress() const;
		const char * getServerName() const;
		const char * getCurrentCommand() const;
	//	std::unordered_map<int, struct CommandStatus *> getCommands() const;
		ClientCLReader * getSender() const;
		ClientReceiver * getReceiver() const;
		Semaphore * getOutputMutex() const;
		std::queue<char *> * getOutputQueue() const;

		void setCurrentCommand(const char *);
		void setServerAddress(struct addrinfo *);	
		void setTerminateAddress(struct addrinfo *);
		void setServerName(const char *);
	//	void setCommands(std::unordered_map<int, struct CommandStatus *>);
		void setReceiver(ClientReceiver *);
		void setSender(ClientCLReader *);
		void setOutputMutex(Semaphore *);
		void setOutputQueue(std::queue<char *> *);

		void addQueueMessage(char *);
		int flushOutputQueue();

		void makeConnection();
		void makeConnection(int, struct addrinfo *);
		void startSession();
		void shutDown();

		void requestOutput(const char *);

		void threadCheckout(Threadable *);
		void closeConnection(int);
		void requestAccess(const char *);
		void relenquishAccess(const char *);
	

		static addrinfo * getAddressInfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
		
};

#endif