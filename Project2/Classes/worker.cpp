#include "../Headers/worker.h"

#include "file_receiver.cpp"
#include "file_sender.cpp"
//#include "file_receiver.cpp"
//#include "file_sender.cpp"
/**
*	Definition of method inherited from Threadable
*	execute() will be executed while in a thread
*	state
**/
void Worker::execute()
{
	doWork();
	//printf("Worker [%d] is done with work\n", (*getId()));
	getNode()->threadCheckout(this);
	finishwork(0);
}

void Worker::fallbackCommandHandle(const char * fullCommand)
{
	char commandName[WORKER_MAX_BUFFER];
	char args[WORKER_MAX_BUFFER];

	memset(&commandName, '\0', sizeof(commandName));
	memset(&args, '\0', sizeof(args));

	extractCommand(fullCommand, commandName, WORKER_MAX_BUFFER);
	extractArgs(fullCommand, args, WORKER_MAX_BUFFER);

	//printf("Worker::fallbackCommandHandle (%d), command: %s, args: %s\n", *getId(), commandName, args);
	if(isQuitCommand(commandName))
	{
	//	finishwork(0);	
		setDone(true);	
	}
	else if (isDeleteCommand(commandName))
	{
		del(args);
	}
	else if (isCdCommand(commandName)) 
	{
		cdr(args);
	}
	else
	{
		useBash(commandName, args);
	}

}

//execute commandName and args and put output in dest
void Worker::useBash(char * commandName,  char * args)
{
	//we can just use execl after getting the args
	pid_t pid;
	int status;

	char * fullCommand = (char *) malloc(snprintf(NULL, 0, "%s %s", commandName, args) + 1);
	sprintf(fullCommand, "%s %s", commandName, args);

	memset(&pid, 0, sizeof(pid_t));
	memset(&status, 0, sizeof(int));

	getNode()->acquireDirectoryMutex();
	updateWorkingDirectory();
	//we need to use fork so we do not overwrite the current process
	//that useBash() was called within
	pid = fork();
	if(pid < 0)
	{
		//fork() failed
		perror("fork");
		status = -1;
	//	finishwork(status);
		setDone(true);
	}
	else if(pid == 0){
		//child
		int error = execl(Worker::shell, Worker::shell, "-c",  fullCommand, (char *)0);
	
		if(error == -1)
		{
			perror("exec");
			_exit(1);
		}
		_exit(0);
	}
	else
	{
		//parent
		//wait for the child to finish execution
		if(waitpid(pid, &status, 0) != pid)
		{
			//error
			perror("waitpid");
			status = -1;
			setDone(true);
		}
	}
	getNode()->releaseDirectoryMutex();

	delete fullCommand;
}


void Worker::cdr(const char *path)
{
	//another thread may have changed the working directory
	//so restore it to this thread's
//	getNode()->getMutex()->p();
	//getNode()->acquireMutex();
	getNode()->acquireDirectoryMutex();
	updateWorkingDirectory();

	int res = chdir(path);
	//res is 0 if success or -1 if fail
	if(res == 0)
	{	
		//success, set the Threads current dir
		setCwd(get_current_dir_name());
	}
	else if (res < 0)
	{
		perror("chdir");
	}
	//getNode()->getMutex()->v();
//	getNode()->releaseMutex();
	getNode()->releaseDirectoryMutex();
	//open(path, O_RDWR);
	//printf("cd to: %s\n", path);
}

void Worker::del(char *filename)
{
	getNode()->releaseMutex();
	getNode()->requestAccess(filename);
	struct stat s;
	int type = stat(filename, &s);

	//char * directive;
	//memset(&directive, '\0', sizeof(directive));

	char command[256];
	if(S_ISREG(s.st_mode))
	{
		//regular file | use rm
		//strcpy(directive, "rm");
		char directive[] = "rm";
		useBash(directive, filename);
	}
	else if (S_ISDIR(s.st_mode))
	{
		//directory, use rmdir
		//strcpy(directive, "rmdir");
		char directive[] = "rmdir";
		useBash(directive, filename);
	}
	getNode()->relenquishAccess(filename);
	getNode()->acquireMutex();
}

/**/
void Worker::quit()
{
	//send quit to the remote
	const char * msg = "quit";
	send(getSocket(), msg, sizeof(msg), 0);
	//finishwork(0);	
	setDone(true);
}

void Worker::addCommandId(int id) 
{
	getCommandIds().push_back(id);
}

int Worker::getSocket() const 
{
	return socket;
}

int Worker::getBytesSent() const 
{
	return bytesSent;
}

int Worker::getBytesReceived() const
{
	return bytesReceived;
}

sockaddr * Worker::getRemoteAddress() const 
{
	return remoteAddress;
}

const char * Worker::getCommand() const
{
	return command;
}

Node * Worker::getNode() const
{
	return node;
}

std::vector<int> Worker::getCommandIds() const
{
	return commandIds;
}



int Worker::getActiveCommandId() const
{
	return activeCommandId;
}

void Worker::setCommand(const char * c)
{
	command = c;
}

void Worker::setSocket(int fd)
{
	socket = fd;
}

void Worker::setRemoteAddress(sockaddr * remote)
{ 
	remoteAddress = remote;
}

void Worker::setBytesSent(int bytes)
{
	bytesSent = bytes;
}

void Worker::setBytesReceived(int bytes)
{
	bytesReceived = bytes;
}

void Worker::setNode(Node * n)
{
	node = n;
}

void Worker::setCommandIds(std::vector<int> ids)
{
	commandIds = ids;
}

void Worker::setActiveCommandId(int d)
{
	activeCommandId = d;
}

/***
*	Extract a the command from src and put it into dest
****/
void Worker::extractCommand(const char * src, char * dest, int size)
{
	//printf("Worker::ExtractCommand... letters: %s\n", letters);

	int len = strlen(src);
	char extractedCommand[len];

	int i = 0;
	while(i < len && i < size)
	{
		char c = src[i];
		if(c == ' ' || c == '\0')
		{
			break;
		}
		dest[i] = src[i];
		i++;
	}
	
}

void Worker::extractArgs(const char * src, char * dest, int size)
{
	int len = strlen(src);

	int i = 0;
	//scan until the first ' '
	while(src[i] != ' ' && i < len) i++;
	i++; //move i past ' '
	//we are now pointing to the args
	//scan them in
	int j = 0;
	while(i < len && j < size)
	{
		dest[j] = src[i];
		i++;
		j++;
	}
}


//rollback?
void Worker::rollback(int commandId) 
{

}

//check the status of the command with id = d
//every 1000 bytes transmitted
bool Worker::commandIsTerminated(int d)
{
	return getNode()->pollCommand(d);
}
/*
bool Worker::isServer() const
{
	return serverWorker;
}
*/
//return true if the file exists, false otherwise
bool Worker::validFile(const char * path)
{
	bool iv = true;

	int res = access(path, R_OK);

	if(res < 0)
	{
		iv = false;
	}

	return iv;
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

bool Worker::isTerminateCommand(const char * command)
{
	if(Worker::terminate_command != command)
	{
		return false;
	}

	return true;
}

void Worker::rollback()
{

}

void Worker::handleError(const char * msg)
{
	perror(msg);
	exit(1);
}
void Worker::finishwork(int status)
{
	//printf("Worker %d is terminating...\n", *getId());
	//we need to tell this Worker's node to remove this thread
	try{
		getNode()->getThreadController()->removeThread(*getId());
	} 
	catch(ThreadDoesNotExistException * e)
	{
	//	printf("thread dne\n");
	}
	//now we need to call the node's closeConnection method
	getNode()->closeConnection(getSocket());
	//call the Threaded interface's finish() method
	finish();
}