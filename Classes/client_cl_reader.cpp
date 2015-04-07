//client_cl_reader.cpp

#include "../Headers/client_cl_reader.h"
//#include "sender.cpp"
//receiver.cpp used sender.cpp, so just let that include it
//#include "receiver.cpp"


void ClientCLReader::doWork()
{
	Receiver * receiver = new Receiver(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());
	FileReceiver * fileReceiver = new FileReceiver(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());
	FileSender * fileSender = new FileSender(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());

	Client * master = (Client *) getNode();

	receiver->setId(getId());
	int bytesReceived = 0;
	int commandId;
	char command[WORKER_MAX_BUFFER];
	char args[WORKER_MAX_BUFFER];
	char entered[WORKER_MAX_BUFFER];
	char commandIdBuf[60];
	bool withThread = false;
	bool isGet = false;
	bool isPut = false;
	bool validCommand = true;
	char received[WORKER_MAX_BUFFER];
	
	//pthread_t receiver_id = * master->getReceiver()->getId();
	//master->getThreadController()->euthanize(receiver_id);//we'll wake it up when we need it

	//not done is provided by the super super class: Worker
	while(!isDone())
	{
	//	printf("ClientCLReader::doWork() ... my id is %d\n", (*getId()));
		memset(&received, '\0', sizeof(received));
		memset(&command, '\0', sizeof(command));
		memset(&args, '\0', sizeof(args));
		memset(&entered, '\0', sizeof(entered));
	//	printf("ClientCLReader::doWork()[%d] ... memset everything done\n", (*getId()));
		//entered was created with malloc in prompt()
		//do not forget to delete itcd deleted	
		prompt(entered, sizeof(entered));
		
		if(strlen(entered) == 0) continue;
		//we have to extract the command from the entered
		//bad naming on my part, I know, but basically
		//the user enters some text like "cd .."
		//cd should be the command and .. the 
		//args. extractCommand will pull 
		//out the cd for us
		extractCommand(entered, command, sizeof(command));
		extractArgs(entered, args, sizeof(args));
		
	//	printf("ClientCLReader::doWork()[%d] ... com: [%s], args: [%s]\n", (*getId()), command, args); 
		
		std::vector<std::string> splitted = Util::split(' ', entered);

		if(splitted.back()[0] == '&')
		{
	//		printf("ClientCLReader::doWork() (%d) ... with thread is true\n", (*getId()));
			splitted.erase(splitted.end());
			memset(&args, '\0', sizeof(args));
			strcpy(args, splitted.back().c_str());
			memset(&entered, '\0', sizeof(entered));
			strcpy(entered, command);
			strcat(entered, " ");
			strcat(entered, args);
			withThread = true;
		}

		isPut = isPutCommand(command);
		isGet = isGetCommand(command);

	//	printf("ClientCLReader::doWork() ... entered [%s], args: [%s]\n", entered, args);
		//worker super class provides this implementation
		setCommand(entered);
		if(isGet)
		{
			//master->getThreadController()->euthanize(receiver_id);
			//yield();
		}
		if(isPut)
		{
			//if it is a put command then we will need to make sure the file exists
			validCommand = validFile(args);
		}
		else if (isTerminateCommand(command))
		{
			//first we have to terminate the command to this cl_reader's master client
			int commandId = Util::parseInt(args);
			master->markCommand(commandId); 
			//master->getThreadController()->euthanize(*fileReceiver->getId());
			terminateCommand(args);
			//master->getThreadController()->wake(*fileReceiver->getId());
			continue;
		}
		//go ahead and tell the Receiver the command that was entered

		if(!validCommand)
		{
			validCommand = true; //reset it for next iter
	//		printf("Invalid command entered [%s]\n", entered);
			continue; //if the command is not valid then take it from the top
		}
		
		//if we are here then it is a valid command
		//notify the receiver of the command
		transmit(entered);

		if(isPut || isGet)
		{
			//we will want to turn off the ClientReceiver for this
		//	master->getThreadController()->euthanize(receiver_id);

			//get the commandId
			memset(&commandIdBuf, '\0', sizeof(commandIdBuf));
			receiver->receive(commandIdBuf, sizeof(commandIdBuf));

			//master->getThreadController()->wake(* master->getReceiver()->getId());

			int commandId = Util::parseInt(commandIdBuf);

			char * idMessage = (char *) malloc(14 + strlen(commandIdBuf) + 1);
			strcpy(idMessage, "Command Id: ");
			strcat(idMessage, commandIdBuf);
			strcat(idMessage, "\n");
			master->requestOutput(idMessage);
			delete idMessage;
			//add the command to the client's command map
			master->addCommand(commandId, entered, (*getId()));
			setActiveCommandId(commandId);

	//		printf("ClientCLReader::doWork() (%d) ... activeCommandId [%d]\n", (*getId()), getActiveCommandId());
			if(isPut)
			{
				
			//	master->getThreadController()->wake(receiver_id);
				//ClientCLReader extends Sender,
				//so when a Sender is told to preform a put command:
				//	this means that we want to send the file denoted by filename to the remoteAddress
				//const char * filename = Util::split('/', args).back();
				if(withThread)
				{
					fileSender->setFileName(args);
					fileSender->setActiveCommandId(getActiveCommandId());
					master->getThreadController()->newThread(fileSender);
				}
				else
				{
					sendFile(args);
				}
			}
			else if (isGet)
			{
				//master->getThreadController()->euthanize(* master->getReceiver()->getId());
				//ClientCLReader extends Sender,
				//so when a sender receives a Get command, that means we will want to receive a file
				//to do this use the Receiver class
				if(withThread)
				{
					fileReceiver->setFileName(args);
					fileReceiver->setActiveCommandId(getActiveCommandId());
					getNode()->getThreadController()->newThread(fileReceiver);
				}
				else
				{
					bytesReceived = receiver->recvFile();
					//bytesReceived is the amount of bytes the receiver just received
					setBytesReceived(getBytesReceived() + bytesReceived);
				}
			}
			if(!withThread)
			{
				//wake the receiver back up
			//	master->getThreadController()->wake(receiver_id);
			}
		}
		
		else if (isQuitCommand(command))
		{
			//we have to kill the client's receiver too
			//pthread_t * receiverId = master->getReceiver()->getId();
			//master->getThreadController()->killGracefully(*receiverId);
			finishwork(0);
		}
		else
		{
			//nap until the receiver wakes us up
		//	master->getThreadController()->wake(receiver_id);
			//nap();	
			master->flushOutputQueue();
			memset(&received, '\0', sizeof(received));
			receiver->receive(received, sizeof(received));
			master->requestOutput(received);
		//	master->getThreadController()->euthanize(receiver_id);
		}

		//delete entered because it was created with malloc
		//delete entered;
		withThread = false;
	}

	//delete[] receiver;

}

void ClientCLReader::prompt(char * buf, int size)
{
	//printf("-- top prompt --\n");
	((Client *) getNode())->flushOutputQueue();

	((Client *) getNode())->requestOutput(PROMPT_TEXT);
	
	std::cin.clear();
	std::cin.getline(buf, size);
	
//	printf("-- bottom prompt --\n");

}
