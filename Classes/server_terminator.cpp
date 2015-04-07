//server_terminator.cpp

#include "../Headers/server_terminator.h"
#include "terminate_receiver.cpp"

Server * ServerTerminator::getSlave() const
{
	return slave;
}

void ServerTerminator::setSlave(Server * s) 
{
	slave = s;
}
//by this point we will be bound to some port on some socket
//we now have to wait for a connection 
//and accept
void ServerTerminator::wait()
{
	printf("ServerTerminator::wait() ... *top*\n");

	int newSocket;
	socklen_t addrSize;

	addrSize = sizeof(getRemoteAddress());
	if((newSocket = accept(getSocket(), (struct sockaddr *) getRemoteAddress(), &addrSize)) == -1)
	{
		handleError("accept");
	}
	//newSocket = NULL;
	//chdir(getRootDirectory());
	printf("ServerTerminator::wait() ... accepted new connection\n");

	getThreadController()->newThread(new TerminateReceiver(newSocket, (struct sockaddr *) getRemoteAddress(), getSlave()));
	//upon accepting a connection we will want to receive the command id to terminate

}


void ServerTerminator::run()
{
	setUp();
	while(true)
	{
		wait();
	}
}