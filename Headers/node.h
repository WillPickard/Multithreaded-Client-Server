//abstract class for a node like a server or client
#ifndef NODE_H
#define NODE_H

#include "../Classes/util.cpp"

#include "../Classes/semaphore.cpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iterator>
#include <unordered_map>
#include "../Classes/thread_controller.cpp"

//#include "../Classes/worker.cpp"

struct CommandStatus
{
//	bool complete;
	bool active;
	bool toTerminate;
	const char * command;
//	const char * args;
	const char * filePath;
	const char * fileName;
	pthread_t threadId;
};

class Node {
	private:
		int socketDomain;
		int socketType;
		int socketProtocol;
		int portNumber;
		int terminatePortNumber;

		int socketFileDescriptor;
		int terminateSocket;
	//	int fileSocket;

		struct sockaddr * remoteAddress;
		struct sockaddr * terminateAddress;
	//	struct sockaddr * fileSocket;

		ThreadController * threadController;
		const char * rootDirectory;

		std::unordered_map<int, struct CommandStatus *> * commands;

		Semaphore * mutex;
		bool mutexOpen;

		Semaphore * directoryMutex;
	protected:	
		//only allow members of inherited objects to
		//call handleError and only allows those 
		//functions to do it from within public
		//or private member functions
		//void handleError(const char * msg)
		//{
		//	perror(msg);
		//	exit(1);
		//}
		Semaphore * getMutex() const
		{
			return mutex;
		}

		Semaphore * getDirectoryMutex() const
		{
			return directoryMutex;
		}

	public:
		void handleError(const char * msg)
		{
			perror(msg);
			exit(1);
		}

		//all inheriting classes must
		//implement run... or else
		virtual void run() = 0;
		virtual void threadCheckout(Threadable *) = 0;
		/** close the connection by getting a socket fd**/
		virtual void closeConnection(int) = 0;
		virtual void requestAccess(const char *) = 0;
		virtual void relenquishAccess(const char *) = 0;

		Node(int port, int sd, int st, int sp):
			portNumber(port),
			socketDomain(sd),
			socketType(st),
			socketProtocol(sp),
			mutexOpen(true)
			{
				setMutex(new Semaphore(1));
				setDirectoryMutex(new Semaphore(1));
				setThreadController(new ThreadController());

				setRootDirectory(get_current_dir_name());
			
				commands = new std::unordered_map<int, struct CommandStatus *>();
			}

		Node(int port, int sd, int st, int sp, int tpn):
			portNumber(port),
			socketDomain(sd),
			socketType(st),
			socketProtocol(sp),
			terminatePortNumber(tpn),
			mutexOpen(true)
			{
				setMutex(new Semaphore(1));
				setDirectoryMutex(new Semaphore(1));
				setThreadController(new ThreadController());
			
				commands = new std::unordered_map<int, struct CommandStatus *>();

			}

		int createSocket() 
		{
			//int fd = socket(getSocketDomain(), getSocketType(), getSocketProtocol());
			int fd = socket(getSocketDomain(), getSocketType(), getSocketProtocol());

			if(fd == -1)
			{
				handleError("socket");
			}

			return fd;
		}

		int addCommand(const char * command, pthread_t id)
		{
			//printf("\tadd command [%s]\n", command);
			getMutex()->p();

			int size = getCommands()->size() + 1;
			//the size will be the id we use

			getMutex()->v();
			int commandId = addCommand(size, command, id);

			//printf("\tadd command ... command id [%d]\n", commandId);
			//getMutex()->v();

			return commandId;
		}

		int addCommand(int commandId, const char * command, pthread_t id)
		{
			getMutex()->p();
			//printf("addCommand(int,char,pthread) ... (%d, %s, %d)\n", commandId, command, id);
			struct CommandStatus * cs = new struct CommandStatus;
			
			std::vector<std::string> splitted = Util::split(' ', command);

			//check if it is put or get
			if((splitted.at(0) == "get" || splitted.at(0) == "get") && splitted.size() == 2)
			{
				std::vector<std::string> filesplit = Util::split('/', splitted.at(1));
				cs->filePath = splitted.at(1).c_str();
				cs->fileName = filesplit.back().c_str();
			}
			else
			{
				cs->filePath = "";
				cs->fileName = "";
			}

			cs->active = true; //it is being executed
			cs->toTerminate = false; //it does not need to be terminated
			cs->command = command; //the actual command
			cs->threadId = id; //the id of the current thread executing this command

			std::pair<int, struct CommandStatus *> pair (commandId, cs);
			//printf(" command size: [%d]\n",getCommands()->size());
			getCommands()->insert(pair); 
			//printf("new command size: [%d]\n",getCommands()->size());
			getMutex()->v();

			return commandId;
		}

		//mark a command for deletion
		bool markCommand(int commandId)
		{
			getMutex()->p();
			
			std::unordered_map<int, struct CommandStatus *>::const_iterator result = getCommands()->find(commandId);

			if(result == getCommands()->end())
			{
				//dne
				getMutex()->v();
				return false;
			}

			//otherwise the command status struct is in the second of the returned result
			struct CommandStatus * cs = result->second;
			cs->toTerminate = true;
		//	printf("Successfully marked command\n");
			getMutex()->v();

			return true;
		}

		//return true if it needs to be deleted
		bool pollCommand(int commandId)
		{
			getMutex()->p();
			bool marked = false;

			std::unordered_map<int, struct CommandStatus *>::const_iterator result = getCommands()->find(commandId);

			if(result != getCommands()->end())
			{
				marked = result->second->toTerminate;
			}

		//	if(marked) printf("command %d is terminated\n", commandId);
		//	else printf("command %d is not terminated\n", commandId);

			getMutex()->v();

			return marked;
		}

		//just mark the command as being no longer active
		void finishCommand(int commandId)
		{
			getMutex()->p();

			std::unordered_map<int, struct CommandStatus *>::const_iterator result = getCommands()->find(commandId);

			if(result != getCommands()->end())
			{
				result->second->active = false;
			}

			getMutex()->v();
		}

		/* GETTERS */
		int getSocketDomain() const
		{
			return socketDomain;
		}

		int getSocketType() const 
		{
			return socketType;
		}

		int getSocketProtocol() const 
		{
			return socketProtocol;
		}

		int getSocket() const
		{
			return socketFileDescriptor;
		}

		int getTerminateSocket() const
		{
			return terminateSocket;
		}

		int getPortNumber() const
		{
			return portNumber;
		}

		ThreadController * getThreadController() const
		{
			return threadController;
		}

		struct sockaddr * getRemoteAddress() const
		{
			return remoteAddress;
		}

		const char * getRootDirectory() const
		{
			return rootDirectory;
		}

		int getTerminatePortNumber() const
		{
			return terminatePortNumber;
		}

		bool mutexAvailable()
		{
			return mutexOpen;
		}

		void acquireMutex() 
		{
			getMutex()->p();
			mutexOpen = false;
		}

		void releaseMutex()
		{
			getMutex()->v();
			mutexOpen = true;
		}

		void acquireDirectoryMutex()
		{
			getDirectoryMutex()->p();
		}

		void releaseDirectoryMutex()
		{
			getDirectoryMutex()->v();
		}

		std::unordered_map<int, struct CommandStatus *> * getCommands() const
		{
			return commands;
		}

		struct sockaddr * getTerminateAddress() const
		{
			return terminateAddress;
		}

		/* SETTERS */
		void setSocketDomain(int sd) 
		{
			socketDomain = sd;
		}

		void setSocketType(int st)  
		{
			socketType = st;
		}

		void setSocketProtocol(int sp)  
		{
			socketProtocol = sp;
		}

		void setSocket(int s) 
		{
			socketFileDescriptor = s;
		}

		void setTerminateSocket(int s)
		{
			terminateSocket = s;
		}

		void setPortNumber(int p)
		{
			portNumber = p;
		}

		void setThreadController(ThreadController * c)
		{
			threadController = c;
		}

		void setRemoteAddress(struct sockaddr * s)
		{
			remoteAddress = s;
		}

		void setRootDirectory(const char * d)
		{
			rootDirectory = d;
		}

		void setTerminatePortNumber(int pn)
		{
			terminatePortNumber = pn;
		}

		void setMutex(Semaphore * s)
		{
			mutex = s;
		}

		void setDirectoryMutex(Semaphore * s)
		{
			directoryMutex = s;
		}

		void setTerminateAddress(struct sockaddr * ta)
		{
			terminateAddress = ta;
		}
};

#endif