//client_receiver.h
#ifndef CLIENT_RECEIVER_ROUTINE_H
#define CLIENT_RECEIVER_ROUTINE_H

//only define client box once
#ifndef CLIENT_BOX_DEFINED
#define CLIENT_BOX_DEFINED

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

#endif

void * clientReceiverRoutine(void * c)
{
	//c will be a ClientBox
	//get the client so we can get the socket
	ClientBox * box = (ClientBox *) c;
	Client * client = box->getClient();

	int bytesRead, totalBytesRead = 0;

	int socket = client->getSocket();

	//buffer for communication
	char buf[CLIENT_RECEIVER_BUFFER_SIZE];

	//create worker to handle specific commands
	Worker * worker = new Worker(socket, (struct sockaddr *) client->getServerAddress());

	//now loopforever, reading data send from the server @getSocket()
	while(true)
	{
		//client->getSemaphore()->p();
		if((bytesRead = recv(socket, buf, sizeof(buf), 0)) > 0)
		{
			//printf("client received: %s\n", buf);
			//printf("client receiver recv");
			//the server sent us something back
			

			totalBytesRead += bytesRead;

			//check to see if it is a get command, in which case we will need to save
			if(strcmp("get", client->getCurrentCommand()) == 0)
			{
				//printf("worker needs to prepare for get\n");
				if(worker->isGetCommand(worker->extractCommand(buf)))
				{
					worker->recvFile(worker->extractArgs(buf));
				}

				//client->setCurrentCommand("")
			}

			else if (strcmp("quit", client->getCurrentCommand()) == 0)
			{
				pthread_exit(0);
			}

			buf[sizeof(buf)] = '\0';
			//print the shit on the screen
			
			printf("%s\n", buf);
			//clear buf
			memset(&buf, 0, sizeof(buf));

			
		}

		else if (bytesRead < 0)
		{
			//error
			//handleError("recv");
			//exit with error
			int error = 1;
			//client->getSemaphore()->v();
			pthread_exit(&error);
		}
		else 
		{
			//closed?
		}
		//client->getSemaphore()->v();
		//pthread_yield();
	}
}

#endif