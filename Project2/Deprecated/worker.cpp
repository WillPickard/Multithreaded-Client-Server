#include "../Headers/worker.h"

/******
*
*
*
*

DEPRECATED ON 01.30.2015 for threadable interface

using namespace std;
//
void * Worker::execute()
{
	//printf("\t\tWorker thread executing... id: %d, command: %s\n", getId(), getCommand());	
	const char * c = extractCommand(getCommand());

	//printf("\t\tExtracted command: %s\n", c);
	//route output to client
	//dup2(getFileDescriptor(), STDOUT_FILENO);
	//dup2(getFileDescriptor(), STDERR_FILENO);
	//check to see if it is a special command
	//printf("top of Worker::executre, the full command is: %s\n", getCommand());
	if(isQuitCommand(c))
	{
		quit();		
	}
	else if (isDeleteCommand(c))
	{
		const char * args = extractArgs(getCommand());
		del(args);
	}
	else if (isGetCommand(c))
	{
		//printf("full command: %s\n", getCommand());
		const char * args = extractArgs(getCommand());
		//printf("get args: %s\n", args);
		recvFile(args);
	}
	else if (isPutCommand(c))
	{
		const char * args = extractArgs(getCommand());
		//printf("put args: %s\n", args);
		sendFile(args);
	}
	else if (isCdCommand(c)) 
	{
		const char * args = extractArgs(getCommand());
		cdr(args);
	}
	else
	{
		useBash(extractArgs(getCommand()));
	}

	//printf("\t\tWorker bottom of execute()\n");
	//close(getFileDescriptor());
}

void Worker::useBash(const char * args)
{
	//we can just use execl after getting the args
	if(fork() == 0){
		//printf("executing %s\n", getCommand());
		dup2(getFileDescriptor(), STDOUT_FILENO);
		dup2(getFileDescriptor(), STDERR_FILENO);
//		printf("calling %s on %s\n", getCommand(), args);
		execl(Worker::shell, Worker::shell, "-c", getCommand(), (const char *)NULL);
	}
}

/****
*
*	Client will type "put <filename>" where filename is the file to save
*	The job of the server will be to store the data it is receiving in get
*	
*	Put will be used to send the data of the file over the socket connection
*
***/
void Worker::put(const char *filename)
{
	//open the file for reading
	std::ifstream file(filename, std::ifstream::in);

	//bool done = false;
	//char buf[512];
	int max = 512;
	//get the size of the file
	//first seek to the very end
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	//now seek to the beginning
	file.seekg(0, file.beg);

	//tell the remote to expect a certain number of bits
	char msg[256] = "[SIZE_DECLARTION=";
	std::stringstream ss;
	ss << fileSize;
	strcat(msg, (const char *) ss.str().c_str());
	strcat(msg,"]");
	//printf("sending : %s\n", msg);
	send(getFileDescriptor(), msg, sizeof(msg), 0);
	/*
	char buf[4];
	recv(getFileDescriptor(), buf, sizeof(buf), 0);
	if(!strcmp(buf, "cool") == 0)
	{
		printf("Fuck\n");
	}
	else
	{
		printf("coolllllllllllllllllllllllllllll\n");
	}
	*/

	//create the buffer to hold the contents of the file
	char fileContents[fileSize];

	//read the file into the fileContents buffer
	file.read(fileContents, fileSize);
	//fileContents[sizeof(fileContents)] = '\0';
	//init variables to keep track of what we are sending
	int bytesSent = 0, totalBytesSent = 0, bufPosition = 0;

	//printf("length of file: %d\n", fileSize);
	//printf("fileContents: \n%s\n", fileContents);
	bytesSent = send(getFileDescriptor(), fileContents, sizeof(fileContents), 0);
	//printf("bytesSent: %d, should send: %d\n", bytesSent, sizeof(fileContents));
	/*
	char * buf;
	//loop until entire file has been sent
	while(totalBytesSent < fileSize)
	{
		if(max > fileSize)
		{
			buf = new char[fileSize];
			//char buf[fileSize];
			printf("sizeof buf vs filesize, %d vs %d\n", sizeof(buf), fileSize);
			//if the size of the buffer is greater than the fileSize, then we can read it and send it in one go
			file.read(buf, fileSize);
			buf[strlen(buf) - 1] = '\0';
		}
		else
		{
			buf = new char[max];
			//if the buffer is smaller, than we only want to read the buffer's size
			file.read(buf, sizeof(buf));
		}

		//now buf has either the entire file or parts of the file inside of it.
		//send it and update totalBytesSent
		bytesSent = send(getFileDescriptor(), buf, sizeof(buf), 0);

		//check for error
		if(bytesSent < 0)
		{
			//error, close file and exit
			file.close();
			return;
		}

		//increment totalBytesSent by what we just sent
		totalBytesSent += bytesSent;
		fileSize -= bytesSent;
		printf("sent %d bytes. totalSent: %d. Total remaining: %d\n", bytesSent, totalBytesSent, fileSize);
	}
	*/
	//send one for empty transfer to tell the server that it's all done
	/**
	char empty[0];
	int sent = send(getFileDescriptor(), empty, 0, 0);
	printf("sent ping: %d\n", sent);
	/**/
	printf("done sending file %s\n", filename);
}

/****
*
*	Client will type "get <filename>" where filename is the name of the file to send
*	The responsibility of the server is to find the file and send it on the socket
*
*****/
void Worker::get(const char *filename, int numBytes)
{
	printf("writing file : %s...\n", filename);

	//we need to filter out the '/' or '\' and return the last string
	int len = strlen(filename);

	printf("len: %d\n", len);

	char newFileName[len];
	//scan until we hit the end or a '\' or '/' in which case, we reset
	int j = 0;
	for(int i = 0; i < len; i++)
	{
		newFileName[j] = filename[i];
		j++;
		if(filename[i] == '\\' || filename[i] == '/')
		{
			memset(&newFileName, '\0', sizeof(newFileName));
			j = 0;
		}
		
	}


	//open the file for writing
	std::ofstream file;
	//ios::in is for input operations, or writing
	file.open(newFileName);
	//wait to recv
	bool done = false; //boolean to control do-while exiting
	char buf[512]; //buffer recv will populate
	int totalBytesRead = 0, bytesRead = 0;

	while(totalBytesRead < numBytes)
	{
		bytesRead = recv(getFileDescriptor(), buf, sizeof(buf), 0);
		//printf("\trecv:\n%s\n", buf);
		totalBytesRead += bytesRead;
		
		//printf("bytesRead: %d, to read: %d\n", totalBytesRead, numBytes);
		
		if(bytesRead < 0)
		{
			//error
			//handle it
		}
		else
		{
			file << buf;
		}
	}
	file.close();	
	printf("done receiving file %s\n", newFileName);
}

/****
*
*	Send the file from local to remote over the socket
*
*****/
void Worker::sendFile(const char *filename)
{
	//printf("sending file: %s\n", filename);
	//printf("command is : %s\n", getCommand());
	//bool doit = false; //bool to control writing the file to disk

	//open the file for writing
	std::ifstream file(filename, std::ifstream::out);

	//check to see if file exists
	if(!file.good())
	{
		dup2(getFileDescriptor(), STDOUT_FILENO);
		dup2(getFileDescriptor(), STDERR_FILENO);
		printf("File %s does not exist.\n", filename);
		file.close();
		return;
	}
	file.close();

	const char * fullCommand = getCommand();

	//send a message through the socket, notifying the other party to prepare for a file
	send(getFileDescriptor(), fullCommand, strlen(fullCommand), 0);
	//now 'put' it on the remote connection
	//file.close();
	put(filename);

}

/****
*
*	recv a file from remote over the socket and save it to disk
*
*****/
void Worker::recvFile(const char *filename)
{
	//printf("receiving file: %s\n", filename);
	//ifstream file(filename, std::ifstream::out);

	/**
	//check to see if the file is "good"
	//or rather if it exists
	if(file.good())
	{
		//if the file exists, then we want to ask the client if they want to overwrite the file
		char msg[256] = "File ";
		strcat(msg, filename);
		strcat(msg, " exists. Do you want to overwrite it? (y/n)");
		send(getFileDescriptor(), *msg, sizeof(*msg), 0);

		char answer[1];
		recv(getFileDescriptor(), answer, )
	}
	/**/

	//just call get(filename)
	//wait for the number of bytes
	char buf[256];
	int bytesRead = recv(getFileDescriptor(), buf, sizeof(buf), 0);

	int len = strlen(buf);
	char res[15];
	for(int i = 0; i < len; i++)
	{
		//printf("%c\n", buf[i]);
		if(buf[i] == '=')
		{
			i++;
			//read til the next close ]
			int j = 0;
			while(buf[i] != ']' && i < len)
			{
				//printf("%c\n", buf[i]);
				res[j] = buf[i];
				j++;
				i++;
			}
		}
	}
	int number;
	istringstream ss(res);
	ss >> number;
	//const char * cool = "cool";
	//send(getFileDescriptor(), cool, sizeof(cool), 0);
	get(filename, number);
}

void Worker::cdr(const char *path)
{
	chdir(path);
	//open(path, O_RDWR);
	//printf("cd to: %s\n", path);
}

void Worker::del(const char *filename)
{
	struct stat s;
	int type = stat(filename, &s);

	char command[256];
	if(S_ISREG(s.st_mode))
	{
		//regular file | use rm
		strcat(command, "rm ");
		strcat(command, filename);
		setCommand(command);
		useBash(filename);
	}
	else if (S_ISDIR(s.st_mode))
	{
		//directory, use rmdir
		strcat(command, "rm ");
		strcat(command, filename);
		setCommand(command);
		useBash(filename);
	}

}

void Worker::quit()
{
	//send quit to the remote
	const char * msg = "quit";
	send(getFileDescriptor(), msg, sizeof(msg), 0);
	terminate(0);	
}

bool Worker::isDone() const
{
	return done;
}

int Worker::getFileDescriptor() const 
{
	return fileDescriptor;
}

int Worker::getTotalBytesSent() const 
{
	return totalBytesSent;
}

int Worker::getTotalBytesReceived() const
{
	return totalBytesReceived;
}

sockaddr * Worker::getRemoteAddress() const 
{
	
}

const char * Worker::getCommand() const
{
	return command;
}

int Worker::getId() const 
{
	return id;
}

void Worker::setId(int d)
{
	id = d;
}

void Worker::setCommand(const char * c)
{
	command = c;
}

void Worker::setFileDescriptor(int fd)
{
	fileDescriptor = fd;
}

void Worker::setRemoteAddress(sockaddr * remote)
{ 
	remoteAddress = remote;
}

void Worker::setTotalBytesSent(int bytes)
{
	totalBytesSent = bytes;
}

void Worker::setTotalBytesReceived(int bytes)
{
	totalBytesReceived = bytes;
}

const char * Worker::extractCommand(const char * letters)
{
	//printf("Worker::ExtractCommand... letters: %s\n", letters);

	int len = strlen(letters);
	char * extractedCommand = (char *) malloc(len);

	for(int i = 0; i < len; i++)
	{
		char c = letters[i];
		if(c == ' ' || c == '\0')
		{
			return ((const char *) extractedCommand);
		}
		extractedCommand[i] = letters[i];
	}
	return ((const char *) extractedCommand);
}

const char * Worker::extractArgs(const char * letters)
{
	int len = strlen(letters);
	char * args = (char *) malloc(len);
	
	//scan until the first space
	int i = 0;
	while(letters[i] != ' ')
	{
		//if it is the end of the line and we have not hit a space, then no args exist
		if(letters[i] == '\0')
		{
			return (const char *)"";
		}
		i++;
	}
	i++; //move it past the space
	
	int j =0;
	for(i; i < len; i++)
	{
		args[j] = letters[i];
		j++;
	}	
	return ((const char *) args);
}


bool Worker::isCommand(const char * command)
{
	int length = sizeof(Worker::commands) / sizeof(*Worker::commands);
	
	bool isCommand = false;
	for(int i = 0; i < length; i++)
	{
		if(Worker::commands[i] == command)
		{
			isCommand = true;
			break;
		}
	}

	return isCommand;
}

bool Worker::isQuitCommand(const char * command)
{
	if(Worker::quit_command != command)
	{
		return false;
	}

	return true;
}

bool Worker::isPutCommand(const char * command)
{
	if(Worker::put_command != command)
	{
		return false;
	}

	return true;
}

bool Worker::isGetCommand(const char * command)
{
	if(Worker::get_command != command)
	{
		return false;
	}

	return true;
}

bool Worker::isCdCommand(const char * command)
{
	if(Worker::cd_command != command)
	{
		return false;
	}
	
	return true;
}	

bool Worker::isDeleteCommand(const char * command)
{
	if(Worker::delete_command != command)
	{
		return false;
	}

	return true;
}



void Worker::handleError(const char * msg)
{
	perror(msg);
	exit(1);
}
void Worker::terminate(int status)
{
	//close(getFileDescriptor());
	done = true;
	//printf("Worker done...\n");
	//printf("Total Bytes Read: %d\n", getTotalBytesReceived());
	//printf("Total Bytes Sent: %d\n", getTotalBytesSent());
	//exit(status);
}