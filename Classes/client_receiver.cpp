//client_receiver.cpp
#include "../Headers/client_receiver.h"

void ClientReceiver::doWork()
{
	//can't get this to work. 
	//fuck it for now
	return;
	char clientReceiverBuffer[WORKER_MAX_BUFFER];
	Client * master = (Client *) getNode();
	pthread_t cl_reader_id = * master->getSender()->getId();
	int br = 0; //bytes read
	while(!isDone())
	{
		//master->getMutex()->p();
		memset(&clientReceiverBuffer, '\0', sizeof(clientReceiverBuffer));
		br = receive(clientReceiverBuffer, sizeof(clientReceiverBuffer));
		if(br > 0)
		{
			master->addQueueMessage(clientReceiverBuffer);
			master->flushOutputQueue();
		}
		
		//printf("ClientReceiver::doWork() [%d] ... waking the thread with id [%d\n", (*getId()), cl_reader_id);
		master->getThreadController()->wake(cl_reader_id);
		
		yield();
	}
}


int ClientReceiver::receive(char * buf, int size)
{
	
	int bytesReceived = 0;
	
	//bytesReceived = recv(getSocket(), buf, size, MSG_DONTWAIT);
	bytesReceived = recv(getSocket(), buf, size, 0);

	if(bytesReceived < 0)
	{
		if(errno == EAGAIN || EWOULDBLOCK)
		{
			//s'all good man
			return 0;
		}
		//error
		//BytesLessThan0Exception is defined in receiver.h
		perror("recv");
		throw new BytesLessThan0Exception();
	}
	else if (bytesReceived == 0)
	{
		//closed connection
		finishwork(0);
	}
	buf[bytesReceived] = '\0';
	setBytesReceived(getBytesReceived() + bytesReceived);

	return bytesReceived;
}
