#include "../Headers/concurrent_delegator.h"
#include "thread_controller.cpp"
#include "../Classes/receiver.cpp"


/****************************
*
*	Below are the method implementations of the ConcurrentDelegator class
*
****************************/

//create the thread controller when it is instantiated
ConcurrentDelegator::ConcurrentDelegator()
{
	setThreadController(new ThreadController());
}

ThreadController * ConcurrentDelegator::getThreadController() const
{
	return threadController;
}

void ConcurrentDelegator::setThreadController(ThreadController * c)
{
	threadController = c;
}
/***
*	@func ConcurrentDelegator::handle
*		- called when a server accepts a connection. This is the concurrentDelegator's implementation of the pure virtual function of the Delegator class
*		- The server will pass handle() a socket descriptor, representing the socket the client is connected to, and the clients address
*
*	@param socketFileDescriptor - fd the Server has accepted a connection to
*	@param remoteAddr - the remote address struct of the client connected on the socketFileDescriptor
*
***/
void ConcurrentDelegator::handle(int socketFileDescriptor, sockaddr * remoteAddr)
{
	//delegate(new Receiver(socketFileDescriptor, remoteAddr));
}

/***
*
*	This is shitty
*	Delegate a connection made by a server to a worker
*	serverReceiverRoutine is a routine where the thread spins forever, waiting to receive input from the client
*	and then executing whatever it receives
*
*****/
void ConcurrentDelegator::delegate(Worker * worker)
{
	try
	{
		getThreadController()->newThread(worker);
	}
	catch(ThreadCreationException & e)
	{
		handleError(e.what());
	}
}

void ConcurrentDelegator::handleError(const char *msg)
{
	perror(msg);
	exit(1);
}