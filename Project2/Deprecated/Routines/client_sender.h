//client_sender.h
#ifndef CLIENT_SENDER_ROUTINE_H
#define CLIENT_SENDER_ROUTINE_H

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

bool prompt(Client * c)
{
	//c->getSemaphore()->p();
	printf("myftp> ");
	//c->getSemaphore()->v();
	return true;
}

void * clientSenderRoutine(void * c)
{
	//c will be a clientbox
	//first get the client from the box so we can use its socket
	ClientBox * box = (ClientBox *) c;
	Client * client = box->getClient();

	int socket = client->getSocket();
	int bytesSent, totalBytesSent = 0;

	char buf[CLIENT_SENDER_BUFFER_SIZE];

	//worker to ahndle specific commands
	Worker * worker = new Worker(socket, (struct sockaddr *)client->getServerAddress());
	//now loop forever, reading information from the comamand line and sending it to the server
	//@ client->getSocket()

	while(prompt(client), std::cin.getline(buf, sizeof(buf)))
	{
		//client->getSemaphore()->p();
		const char * command = worker->extractCommand(buf);
		const char * args = worker->extractArgs(buf);

		//printf("client_sender routine, command: %s, args: %s\n", command, args);
		client->setCurrentCommand(command);

		//if the user entered the put command, we will have to send it 
		//because this worker was originally implemented for use 
		//as a helper to a Server object, when the client wants
		//to 'put' or send a file, it must use get
		//this is because to the Server, get() means 'find a file in my filesystem', and give it to the client
		//To the client, get() can be thought of as 'find a file in my filesystem', and give it to the server
		if(worker->isPutCommand(command))
		{
			//printf("worker in clientSenderRoutine() thinks it is a put comand\n");
			//we need to reformat the command
			char newCommand[CLIENT_SENDER_BUFFER_SIZE] = "get ";
			strcat(newCommand, args);
			//printf("worker thought it was put, reformatted to %s\n", newCommand);
			//set the command
			//printf("newCommand: %s\n", newCommand);
			worker->setCommand((const char *)newCommand);
			//tell the server that it needs to prepare for put
			/**
			int result = send(socket, buf, sizeof(buf), 0);
			if(result == -1)
			{
				pthread_exit(&result);
			}
			/**/

			//now send the file to the server
			//args = worker->extractArgs(newCommand);
			//printf("............ worker extractArgs: %s\n",args);
			
			worker->sendFile(args);
		}

		else if (worker->isQuitCommand(command))
		{
			send(socket, buf, sizeof(buf), 0);
			//client->getSemaphore()->v();
			pthread_exit(0);
		}
	
		//if it is not put or get then send it to the server and let
		//the server worry about what to do
		else{
			if((bytesSent = send(socket, buf, sizeof(buf), 0)) == -1)
			{
				//error bro
				//handleError("send");
				//exit with error
				int error = 1;
				pthread_exit(&error);
			}

			totalBytesSent += bytesSent;
		}

		//client->getSemaphore()->v();
		//pthread_yield();

	}
}

#endif