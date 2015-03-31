#include "../Headers/server.h"
//the server_receiver routine must come before the concurrent_delegator
//#include "../Routines/server_receiver.h"
//#include "concurrent_delegator.cpp"
#include "receiver.cpp"

Server::Server(int portN, int domain, int type, int protocol = 0)
	: Node(portN, domain, type, protocol)
	{
		//create server address
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = domain;
	//	printf("Server::createBind() ... serv_addr.sin_family: %d\n", serv_addr.sin_family);
		serv_addr.sin_port = htons(portN);
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		//inet_pton(domain, "127.0.0.1", &serv_addr.sin_addr);
		memset(serv_addr.sin_zero, '\0', sizeof serv_addr.sin_zero);

		//printf("rootdir: [%s]\n", getRootDirectory());
		setServerAddress(serv_addr);

		setFileMap(new std::unordered_map<int, Semaphore *> ());
		setFileMapMutex(new Semaphore(1));
	}

Server::Server(int portN, int domain, int type, int protocol, int terminatePort):
	Server(portN, domain, type, protocol)
	{
		setTerminatePortNumber(terminatePort);
		//printf("terminatePortNumber: %d\n", terminatePort);
		
		/**/
		struct sockaddr_in term_addr;
		memset(&term_addr, 0, sizeof(term_addr));

		term_addr.sin_family = domain;
		term_addr.sin_port = htons(terminatePort);
		term_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		//inet_pton(domain, "127.0.0.1", &term_addr.sin_addr);
		memset(term_addr.sin_zero, '\0', sizeof term_addr.sin_zero);

		setTerminateAddress(term_addr);
		/**/
	};


struct sockaddr_in  Server::getServerAddress() const
{
	return serverAddress;
}


ConcurrentDelegator * Server::getDelegator() const
{
	return delegator;	
}

int Server::getTerminateSocket() const
{
	return terminateSocket;
}

struct sockaddr_in Server::getTerminateAddress() const
{
	return terminateAddress;
}

std::unordered_map<int, Semaphore *> * Server::getFileMap() const
{
	return fileMap;
}

Semaphore * Server::getFileMapMutex() const
{
	return fileMapMutex;
}


void Server::setServerAddress(struct sockaddr_in sa)
{
	serverAddress = sa;
}

void Server::setDelegator(ConcurrentDelegator * d)
{
	delegator = d;
}

void Server::setTerminateSocket(int s)
{
	terminateSocket = s;
}

void Server::setTerminateAddress(struct sockaddr_in s)
{
	terminateAddress = s;
}

void Server::setFileMap(std::unordered_map<int, Semaphore *> * m)
{
	fileMap = m;
}

void Server::setFileMapMutex(Semaphore * s)
{
	fileMapMutex = s;
}


/****
* Bind the file descriptor returned from createSocket and bind
* it to a port number
****/
void Server::createBind()
{
	struct sockaddr_in sa = getServerAddress();
//	printf("Server::createBind() ... serv_addr.sin_family: %d\n", sa.sin_family);
	if(bind(getSocket(), (struct sockaddr *) &sa, sizeof(sa)) == -1)
	{
		printf("Server::createBind() failed\n");
		handleError("bind");
	}
}

/***
*
* Create a new thread to handle the request on socketfd from remoteAddr
*
***/
void Server::delegate(int socketfd, sockaddr * remoteAddr)
{
	//getDelegator()->handle(socketfd, remoteAddr);
	getThreadController()->newThread(new Receiver(socketfd, remoteAddr, this));
}


/***
* After socket() and bind(),
*	Should be placed inside forever loop, wait for the command to come and then delegate it
****/
void Server::wait()
{
	printf("Server::wait() ... waiting for connection root dir: [%s]...\n", getRootDirectory());

	int newSocket;
	socklen_t addrSize;

	addrSize = sizeof(getRemoteAddress());
	if((newSocket = accept(getSocket(), (struct sockaddr *) getRemoteAddress(), &addrSize)) == -1)
	{
		handleError("accept");
	}
	//newSocket = NULL;
	acquireDirectoryMutex();
	printf("changing directory to %s\n", getRootDirectory());
	chdir(getRootDirectory());
	printf("Server::wait() ... Current threads: %d, accepting new connection ... delegating...\n", getThreadController()->numThreads());
	delegate(newSocket, (struct sockaddr *) getRemoteAddress());
	releaseDirectoryMutex();
	//getDelegator()->handle(newSocket, (sockaddr *)getRemoteAddress());

}
/*
* creates a socket and binds it to the port
*	Then calls listen
*/
void Server::setUp()
{
	//set up the delegator
	//setDelegator(new ConcurrentDelegator());
	//printf("Server::setUP ... rootDir: [%s]\n", getRootDirectory());
	//printf("Server::setUP ... rootDir: [%s]\n", getRootDirectory());
	//socket
	printf("Creating socket...\n");
	int fd = createSocket();
	setSocket(fd);
//	printf("Server::setUp() ... socket: %d\n", fd);
	//bind
	printf("Binding...\n");
	createBind();

	//printf("--rootdir: [%s]\n", getRootDirectory());

	//printf("--rootdir: [%s]\n", getRootDirectory());
	if(listen(getSocket(), Server::backlog) == -1)
	{
		handleError("listen");
	}
}

/**
*	Must implement threadCheckout: inherited from Node
**/
void Server::threadCheckout(Threadable * t)
{

}

/**
*	Inherited from Node as pure virtual
*	When a thread in the server wants to close its connection
*	it is because the connection with the client is over
*	In which case we can close the socket
**/
void Server::closeConnection(int socket)
{
	close(socket);
}
/***
*	Static method run()
*		Creates a server object, then calls setUp()
*		setUp() will create a socket and bind, setting the appropriate variables
*		it will then listen until a connection is made, at which time it will delegate
****/
void Server::run()
{
	//printf("Server::run() ... root dir: [%s]\n", getRootDirectory());
	//printf("Server::run() ... root dir: [%s]\n", getRootDirectory());
	setUp();
	while(true)
	{
		wait();
	}

	printf("Server is exiting. Goodbye\n");
}

//threads will ask the server if they can do operations on a filename
void Server::requestAccess(const char * filename)
{
	getFileMapMutex()->p();
	
	int hash = hashFileName(filename);
	//we need to take ownership of the string to malloc it
//	std::string * myfn = new std::string (filename);

	std::unordered_map<int, Semaphore *>::const_iterator result = getFileMap()->find(hash);

	if(result == getFileMap()->end())
	{
		//the filename dne yet
		printf("[%s] dne, creating...\n", filename);
		Semaphore * s = new Semaphore(1);
		std::pair<int, Semaphore *> pair (hash, s);
		getFileMap()->insert(pair);
		s->p();//acquire it, we'll get it because it is init to 1
		getFileMapMutex()->v();
	}
	else
	{
		//it does exist, so try and acquire the semaphore after releasing the mutex
		printf("[%s] exists, attempting to acquire mutex\n", filename);
		getFileMapMutex()->v();
		result->second->p();
		printf("[%s], acquired mutex, returning...\n", filename);
	}

}

void Server::relenquishAccess(const char * filename)
{
	getFileMapMutex()->p();
	//std::string * myfn = new std::string (filename);
	int hash = hashFileName(filename);
	std::unordered_map<int, Semaphore *>::const_iterator result = getFileMap()->find(hash);

	if(result == getFileMap()->end())
	{
		//?wut, if a thread is relenquishing access, then it is assumed it has possession
		printf("attempted to relenquish access to [%s], but it did not exist!\n", filename);
	}
	else
	{
		//it does exist, relenquish the semaphore
		printf("[%s], relenquishing access\n", filename);
		result->second->v();
	}

	getFileMapMutex()->v();
}

int Server::hashFileName(const char * fn)
{
	std::vector<std::string> splitted = Util::split('/', fn);

	const char * s = splitted.back().c_str();
	int hash = 0;
	int len = strlen(s);
	for(int i = 0; i < len; i++)
	{
		//printf("\t[%c] -> [%d]\n", s[i], s[i]);
		hash += (int) s[i];
	}

	//printf("\tResult hash: [%d]\n", hash);
	return hash;
}
