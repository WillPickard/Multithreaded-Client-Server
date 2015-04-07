//sender.cpp

#include "../Headers/sender.h"


//implementation of Super class doWorkker's pure virtual
void Sender::doWork()
{

}

//send the file as filename
//return the amount of bytes sent
int Sender::sendFile(const char * filename)
{
	getNode()->requestAccess(filename);
	
	int bytesSent = 0;
	int totalBytesSent = 0;
	int bytesRemaining;
	int fileSize = 0; //size of file is bytes
	int expectedSize = 0;//account for overhead
	
	//buffer we will read into
	char buf[WORKER_MAX_BUFFER];
	memset(&buf, '\0', sizeof(buf));
	
	//attempt to open the file denoted by filename
	std::ifstream file (filename, std::ifstream::binary);
	
	if(!file.is_open())
	{	
		//oh shit, file dne, do something!
		printf("Sender::sendFile() (%d) ... the file: [%s] does not exist!\n", (*getId()), filename);
		//bytesSent = transmit("File does not exist");
		getNode()->relenquishAccess(filename);
		fileSize = expectedSize = 0;
		//return bytesSent;
	}
	else
	{
		//get the filesize
		file.seekg(0, file.end);//seek to end of file
		fileSize = file.tellg();//actual filesize
		file.seekg(0, file.beg);//seek to beginning again
		bytesRemaining = fileSize;
		expectedSize = fileSize + WORKER_MAX_BUFFER; //shittily account for potential overhead
	}
	
	std::vector<std::string> splitted = Util::split('/', filename);
	std::string fileName = splitted.back();

	//wait for the request
	Receiver * r = new Receiver(getSocket(),(struct sockaddr *) getRemoteAddress(), getNode());
	char temp [WORKER_MAX_BUFFER];
	memset(&temp, '\0', sizeof(temp));
	r->setId(getId());
	r->receive(temp, sizeof(temp));
	sendFileInfo(fileName, expectedSize);
	memset(&temp, '\0', sizeof(temp));
	//wait one more time for the remote node to send back a message saying the got iter_swap
	r->receive(temp, sizeof(temp));
	yield();
	
	if(fileSize <= 0){
		return 0;
	}

	bool fuckit = false;
	int check = 0;
	int sizeToSend = 0;
	while(totalBytesSent < fileSize)
	{
		memset(&buf, '\0', sizeof(buf));
		file.read(buf, sizeof(buf));
		
		bytesSent = transmit(buf);
		totalBytesSent += bytesSent;
		check += bytesSent;


		if(check >= WORKER_CHECK_COMMAND)
		{
			//printf("poll-");
			fuckit = commandIsTerminated(getActiveCommandId());
		//	fuckit ? printf("true\n") : printf("false\n");
			check = 0;
		}

		if(fuckit)
		{
			file.close();
			getNode()->relenquishAccess(filename);
			return totalBytesSent;
		}
	}
	
	if(totalBytesSent < expectedSize)
	{
		int diff = expectedSize - totalBytesSent;
		char cleanup[diff];
		for(int i = 0; i < diff; i++)
			cleanup[i] = ' ';
			
		bytesSent = transmit(cleanup);		
		totalBytesSent += bytesSent;
	}

	file.close();

	getNode()->relenquishAccess(filename);
	return totalBytesSent;
}

//transmit some data
//return the amount of bytes sent
int Sender::transmit(const char * data)
{
	int bytesSent = 0;

	bytesSent = send(getSocket(), data, strlen(data), 0);

	if(bytesSent < 0)
	{
		perror("Sender::transmit, bytes less than 0");
		throw new SentLessThan0Exception();
	}

	setBytesSent(getBytesSent() + bytesSent);
	return bytesSent;
}

//send the file size to the receiver
void Sender::sendFileInfo(std::string filename, int filesize)
{
	FileInfo * fi = new FileInfo(filename, filesize);
	const char * msg = fi->serialize().c_str();
	transmit(msg);
}

/** 
* Notify the server that the command needs to be terminated
*	by sending the command id
*
**/
int Sender::terminateCommand(int commandId)
{
	getNode()->markCommand(commandId);

	char tcbuf[WORKER_MAX_BUFFER];
	Util::intToString(commandId, tcbuf);
	return terminateCommand(tcbuf);
}

int Sender::terminateCommand(const char * msg)
{
	int sendResult;

	sendResult = send(getNode()->getTerminateSocket(), msg, strlen(msg), 0);

	if(sendResult < 0)
	{
		throw new SentLessThan0Exception();
	}

	setBytesSent(getBytesSent() + sendResult);
	
	return sendResult;
}
