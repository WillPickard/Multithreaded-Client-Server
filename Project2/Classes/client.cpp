#include "../Headers/client.h"
//#include "worker.cpp"
//#include "../Routines/client_receiver.h" //for the thread which will receive
//#include "../Routines/client_sender.h" // for the thread which will loop and send
//#include "thread_controller.cpp"
//#include "semaphore.cpp"
#include "receiver.cpp"

#include "client_cl_reader.cpp"
#include "client_receiver.cpp"

#include <unordered_map>

Client::Client(int p, int sd, int st, int sp)
	: Node(p, sd, st, sp)
	{
		//mutex
		setOutputMutex(new Semaphore(1));
		setOutputQueue(new std::queue<char *> ());
	};

Client::Client(int p, int sd, int st, int sp, const char * sn)
	: Node(p, sd, st, sp),
	serverName(sn)
	{
		setOutputMutex(new Semaphore(1));
		setOutputQueue(new std::queue<char *> ());
	};
Client::Client(int p, int sd, int st, int sp, const char * sn, int tpn)
	: Client(p, sd, st, sp, sn)
	{
		setTerminatePortNumber(tpn);
	};


/***
*	Static wrapper for c style getaddrinfo call
****/
struct addrinfo * Client::getAddressInfo(const char * name, const char * port, const struct addrinfo * hints, struct addrinfo ** serverAddress)
{
	if(getaddrinfo(name, port, hints, serverAddress) != 0)
	{
		perror("getAddressInfo");
		exit(1);
	}

	return *serverAddress;
}

const char * Client::getCurrentCommand() const
{
	//we need to lock this down
	//calling p() will attempt to decrement the semaphore's internal allowed thread count
	//here we only want one. If a thread calls this, and p = 0, then we will block until signaled
	//getSemaphore()->p();
	
	return currentCommand;

	//allow new thread through
	//getSemaphore()->v();
}

struct addrinfo * Client::getServerAddress() const 
{
	return serverAddress;
}

struct addrinfo * Client::getTerminateAddress() const 
{
	return terminateAddress;
}

const char * Client::getServerName() const
{
	return serverName;
}

/*
std::unordered_map<int, struct CommandStatus *> Client::getCommands() const
{
	return commands;
}
*/

ClientCLReader * Client::getSender() const
{
	return sender;
}

ClientReceiver * Client::getReceiver() const
{
	return receiver;
}

Semaphore * Client::getOutputMutex() const
{
	return outputMutex;
}

std::queue<char *> * Client::getOutputQueue() const
{
	return outputQueue;
}


/** SETTERS **/

void Client::setCurrentCommand(const char * c)
{
	currentCommand = c;
}

void Client::setServerAddress(struct addrinfo * address)
{
	serverAddress = address;
}

void Client::setTerminateAddress(struct addrinfo * address)
{
	terminateAddress = address;
}

/*
void Client::setCommands(std::unordered_map<int, struct CommandStatus *> c)
{
	commands = c;
}
*/


void Client::setSender(ClientCLReader * s)
{
	sender = s;
}

void Client::setReceiver(ClientReceiver * r)
{
	receiver = r;
}

void Client::setOutputMutex(Semaphore * s)
{
	outputMutex = s;
}

void Client::setOutputQueue(std::queue<char *> * q)
{
	outputQueue = q;
}


/***
*	After setting up proper variables, like where to connect to
*	init the connection by creating the socket
*	and then making the connection
***/
void Client::makeConnection()
{
	//create the socket to do shit on
	//remember that it is inherited
	setSocket(createSocket());
	
	printf("Attempting connection...\n");
	if(connect(getSocket(), getServerAddress()->ai_addr, getServerAddress()->ai_addrlen) == -1)
	{
		handleError("connect to server");
	}


	//connect to the terminateport
	if(connect(getTerminateSocket(), getTerminateAddress()->ai_addr, getTerminateAddress()->ai_addrlen) == -1)
	{
		handleError("connect to server terminator");
	}

	
	printf("Connected... normal socket: [%d] | terminate socket: [%d]\n", getSocket(), getTerminateSocket());	
}

/****
*
*	Start sending/receiving data
*
*****/
void Client::startSession()
{
	char * currentCommand = NULL;
	//create a thread to handle receiving

	ThreadController * threader = getThreadController();

	//make two seperate threads to send a receive seperately
	pthread_t sender, receiver;
	int senderStatus, receiverStatus, senderResult, receiverResult;

	ClientCLReader * cl_reader = new ClientCLReader(getSocket(), (struct sockaddr *) getRemoteAddress(), this);
	//ClientReceiver * cl_receiver = new ClientReceiver(getSocket(), (struct sockaddr *) getRemoteAddress(), this);

	//setReceiver(cl_receiver);
	setSender(cl_reader);

	sender = threader->newThread(cl_reader);
	//receiver = threader->newThread(cl_receiver);

	//join to sender and receiver so we wait until both have finished their execution (they never will)
	threader->join(sender);
	//threader->join(receiver);	

	shutDown();
}

void Client::run()
{
	//we need to define the serverAddress
	struct addrinfo hints; //stuff that we know  about the server
	struct addrinfo *sa; //will be filled out with the server

	struct addrinfo term_hints;
	struct addrinfo * term_sa;

	//reset hints
	memset(&hints, 0, sizeof(hints));
	memset(&term_hints, 0, sizeof(term_hints));

	hints.ai_family = getSocketDomain(); 
	hints.ai_socktype = getSocketType(); 
	hints.ai_flags = AI_PASSIVE; //fill my ip for me

	term_hints.ai_family = getSocketDomain();
	term_hints.ai_socktype = getSocketType();
	term_hints.ai_flags = AI_PASSIVE;

	//convert the port number to a const char *
	const char * pn;
	const char * tn;

	//set up the regular server address
	std::stringstream convert;
	convert << getPortNumber();
	std::string s = convert.str();
	pn = s.c_str();

	setServerAddress(Client::getAddressInfo(getServerName(), pn, &hints, &sa));
	
	//set up the terminate address
	convert.str(std::string());
	convert << getTerminatePortNumber();
	s = convert.str();
	tn = s.c_str();
	printf("tn: %s\n", tn);

	setTerminateAddress(Client::getAddressInfo(getServerName(), tn, &term_hints, &term_sa));

	//set the socket for terminating
	setTerminateSocket(createSocket());

	makeConnection();
	startSession();
}


void Client::shutDown()
{
	//kill all running threads
	//getThreadController()->killGracefully((*getReceiver()->getId()));
	getThreadController()->killGracefully((*getSender()->getId()));

	close(getSocket());
	printf("Shutdown... have a nice day :)\n");
}


/***
* Use a mutex to only allow one person to print
*	shit to the screen at a time
***/
void Client::requestOutput(const char * toPrint)
{
	getOutputMutex()->p();

	printf("%s", toPrint);

	getOutputMutex()->v();
}

void Client::addQueueMessage(char * msg)
{
	getOutputMutex()->p();

	//we want to dynamically allocate it
	char * m = (char *) malloc(strlen(msg) + 1);
	strcpy(m, msg);

	getOutputQueue()->push(m);

	getOutputMutex()->v();
}
int Client::flushOutputQueue()
{
	getOutputMutex()->p();

	int n_flushed = 0;
	std::queue<char *> * q = getOutputQueue();

	while(!q->empty())
	{
		char * m = q->front();
		printf("%s", m);
		q->pop();
		delete m;
	}

	//printf("\n");
	getOutputMutex()->v();

	return n_flushed;
}

void Client::threadCheckout(Threadable * t)
{
	pthread_t thread_id = * t->getId();
	
	try
	{
		getThreadController()->removeThread(thread_id);
	}
	catch (ThreadDoesNotExistException * e)
	{
		printf("Client ... tried to remove thread [%d] but it dne\n", thread_id);
	}

	//send a signal to the receiver in case it is sleeping
	//pthread_t receiver_id = * getReceiver()->getId();
	//if(receiver_id != thread_id)
//	{
	//	getThreadController()->wake(receiver_id);
	//}
}

//pure virtual from Node
//The client does not want to close the connection
//	because it has threads regularly executing and finishing
void Client::closeConnection(int socketFd)
{
	//do nothing
}

//just let them through, the client does not have to worry about multiple threads accessing the same file
void Client::requestAccess(const char * filename)
{
//do nothing, this is on purpose
}

void Client::relenquishAccess(const char * filename)
{
//do nothing, this is on purpose
}