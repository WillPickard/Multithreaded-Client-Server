//receiver.cpp

#include "../Headers/receiver.h"
#include "worker.cpp"
#include "sender.cpp"
//#include "file_receiver.cpp"
//#include "file_sender.cpp"

//inherited virtual method from Worker
void Receiver::doWork()
{

	//a Receiver may receive commands to send information back
	//use a sender to do this
	//do not init it b/c maybe we will never need to use it
	//so wait to init it until it is requested, and init it only once
	Sender * sender = new Sender(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());
	FileReceiver * fileReceiver = new FileReceiver(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());
	FileSender * fileSender = new FileSender(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());

	sender->setId(getId());
	char command[WORKER_MAX_BUFFER];
	char args[WORKER_MAX_BUFFER];
	char received[WORKER_MAX_BUFFER];
	char commandIdBuf[WORKER_MAX_BUFFER];
	char response[WORKER_MAX_BUFFER];
	//char filename[WORKER_MAX_BUFFER];

	//for stealing the output of fallback command handle
	int out_pipe[2];
	int err_pipe[2];
	int old_out, old_err;

	bool isPut = false;
	bool isGet = false;
	int commandId;

	pthread_t threadId = (*getId());
	Node * node = getNode();
//	std::vector<const char *> commands; 
	std::vector<const char *> splitted;
	while(!isDone())
	{
		memset(&received, '\0', sizeof(received));
		memset(&command, '\0', sizeof(command));
		memset(&args, '\0', sizeof(args));
		memset(&response, '\0', sizeof(response));

	//	printf("Receiver::doWork() (%d) ... cwd : %s\n", (*getId()), getCwd());
		
		receive(received, sizeof(received));

		//printf("Receiver::doWork() (%d) ... received: [%s]\n", (*getId()), received);
		extractCommand(received, command, sizeof(command));
		extractArgs(received, args, sizeof(args));

	//	printf("Receiver::doWork() (%d) ... command: %s, args: %s\n", (*getId()), command, args);

		isPut = isPutCommand(command);
		isGet = isGetCommand(command);

		if(isPut || isGet)
		{
			//we will need to send the clientId
			memset(&commandIdBuf, '\0', sizeof(commandIdBuf));
			commandId = node->addCommand(received, threadId);
			Util::intToString(commandId, commandIdBuf);
			sender->transmit(commandIdBuf);

			setActiveCommandId(commandId);
			if(isPut)
			{
				//if a Receiver receives a put command:
				//	that means the Sender wants to PUT a file or send a file
				//	we must prepare to receive
				//	the file denoted by args
				//	from the Sender
				recvFile();
			}
			else if (isGet)
			{
				//if a Receiver receives a get command:
				//	That means the Sender is requesting, or wants to GET, the file denoted by args
				//	To do this, we will create the Sender if it has not been created already
				//	then tell it to send the file
				//	Note: 
				//		Unlike when a Client or Server creates a Worker subclass, we will not use a new thread to do this
				//		So we will not return from the call to the Sender's sendFile function until it actually completes
				sender->setActiveCommandId(getActiveCommandId());
				sender->sendFile(args);
			}	
		}
		else
		{
			//CRITICAL SECTION (b/c of routing output)
			//node->getMutex()->p();
			node->acquireMutex();
			//let the kernel sort it out
			//we will want to transmit back to the client, so make a pipe for stdout and stderr where the fallback() func will write to
		//	printf("Receiver::doWork() (%d) ... making pipes\n", (*getId()));
			old_out = dup(STDOUT_FILENO);
			old_err = dup(STDERR_FILENO);
			int r1 = pipe(out_pipe);
			int r2 = pipe(err_pipe);

			if(r1 != 0 || r2 != 0)
			{
				//?
				printf("error creating pipe\n");
			} 
		//	printf("Receiver::doWork() (%d) ... pipes made\n", (*getId()));
			//redirect out and err to their pipes
			dup2(out_pipe[1], STDOUT_FILENO);
			dup2(err_pipe[1], STDERR_FILENO);
			close(out_pipe[1]);
			close(err_pipe[1]);
			//change the read end to non blocking
			int out_e = fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);
			int err_e = fcntl(err_pipe[0], F_SETFL, O_NONBLOCK);

			if(out_e == -1 || err_e == -1)
			{
			//	printf("error making the pipes non block\n");
			}


			fallbackCommandHandle(received);
			fflush(stdout);
			int err_size = 0;
			int out_size = 0;

			if( (err_size = read(err_pipe[0], response, sizeof(response))) > 0)
			{
				sender->transmit(response);
			}

			else if( (out_size = read(out_pipe[0], response, sizeof(response))) > 0)
			{
				sender->transmit(response);
			}

			else
			{
				//no output and no error
				strcpy(response, "\n");
				sender->transmit(response);
			}

			close(out_pipe[0]);
			close(err_pipe[0]);

			
			//restore the pipes
			dup2(old_out, STDOUT_FILENO);
			dup2(old_err, STDERR_FILENO);

			node->releaseMutex();
			//printf("Worker::fallbackCommandHandle() (%d) ... response: [%s]\n", (*getId()), response);
		}
	}//while
	//delete[] sender;
	//delete[] fileSender;
	//delete[] fileReceiver;
	//finishwork(0);
}

//block on socket until we receive something and then fill the buffer and return the number of bytes received
int Receiver::receive(char * buf, int size)
{
	
	//printf("Receiver::receive() [%d] ... top\n", (*getId()));	
	int bytesReceived = 0;
	
	bytesReceived = recv(getSocket(), buf, size, 0);
	
//	printf("Receiver::receive() [%d] ... received: [%s], bytes: [%d]\n", (*getId()), buf, bytesReceived);
	
	if(bytesReceived < 0)
	{
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
	//buf[bytesReceived] = '\0';
	setBytesReceived(getBytesReceived() + bytesReceived);

	return bytesReceived;
}

//recv a file denoted by filename
//return bytes received
//this is kind of tricky in multithreaded
//we will have to make sure that we maintain the correct working directory across all threads
int Receiver::recvFile()
{
	FileInfo * fi = requestFileInfo();
	int fileSize = fi->getFileSize();
	std::string fileName = fi->getFileName();
	
	if(fileName.length() <= 0 || fileSize <= 0)
	{
		printf("Something bad happened. The filename received in confirmation has a length 0 or less!\n");
		return 0;
	}
	//printf("Receiver::recvFile() (%d), filename [%s]\n", (*getId()), fileName.c_str());

	int bytesReceived = 0;
	int totalBytesReceived = 0;
	bool fuckit = false;

	//fileSize = requestFileSize();

	char buf[WORKER_MAX_BUFFER];
	//open the file for writing

	const char * fileName_c = fileName.c_str();
	
	getNode()->requestAccess(fileName_c);
	getNode()->acquireDirectoryMutex();
	updateWorkingDirectory();
	
	std::ofstream file;
	file.open(fileName_c, std::ios::out | std::ios::binary);
	//printf("Receiver::recvFile() (%d), preparing to receive the file: %s ... size: [%d]\n", (*getId()), fileName_c, fileSize);
	
	struct timeval tv;
	fd_set readfds;
	
	FD_ZERO(&readfds);
	FD_SET(getSocket(), &readfds);
	
	tv.tv_sec = 1;
	tv.tv_usec = 250000;
	
	int check = 0;
	while(totalBytesReceived < fileSize)
	{

		select(getSocket()+1, &readfds, NULL, NULL, &tv);
		
		if(FD_ISSET(getSocket(), &readfds)){
			memset(&buf, '\0', sizeof(buf));
			bytesReceived = receive(buf, sizeof(buf));
			
			totalBytesReceived += bytesReceived;
			check += bytesReceived;
			file << buf;
		}	
		else{
			fuckit = commandIsTerminated(getActiveCommandId());
			check = 0;
			if(fuckit)
			{
				file.close();
				int removed = remove(fileName_c);
				if(removed != 0)
				{
					perror("Receiver::recvFile() remove");
				}
				getNode()->releaseDirectoryMutex();
				getNode()->relenquishAccess(fileName_c);
				return totalBytesReceived;
			}
		}
	}
	
	//printf("Done receiving file.\n");
	file.close();
	
	getNode()->releaseDirectoryMutex();
	getNode()->relenquishAccess(fileName_c);
	//delete fi;
	return bytesReceived;
}


FileInfo * Receiver::requestFileInfo()
{
	Sender * s = new Sender(getSocket(), (struct sockaddr *) getRemoteAddress(), getNode());
	s->setId(getId());
	s->transmit("filinforequest");
	
	char requestFileInfoBuf[WORKER_MAX_BUFFER];
	memset(&requestFileInfoBuf, '\0', sizeof(requestFileInfoBuf));

	receive(requestFileInfoBuf, sizeof(requestFileInfoBuf));
	//printf("Receiver::requestFileInfo() ... received: [%s]\n", requestFileInfoBuf);
	FileInfo * fi = new FileInfo(requestFileInfoBuf);
	
	//tell the remote node that we received the info and to proceed
	s->transmit("proceed");
	return fi;
}
