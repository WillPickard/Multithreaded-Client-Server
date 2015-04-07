#ifndef WORKER_H
#define WORKER_H

static const int WORKER_MAX_BUFFER = 512;
static const int WORKER_CHECK_COMMAND = 10000000;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/wait.h>//waitpid

#include "threadable.h" //threading interface

//class Node;
#include "node.h" //shared memory

#ifndef FILE_INFO_CLASS
#define FILE_INFO_CLASS
class FileInfo
{
public:
	std::string fileName;
	int fileSize;

	FileInfo(std::string fn, int s):
		fileName(fn),
		fileSize(s)
		{};

	FileInfo(const char * msg)
	{
		build(msg);
	}

	void build(const char * msg)
	{
		//printf("FileInfo::build() ... [%s]\n", msg);
		std::vector<std::string> splitted = Util::split(' ', msg);
		//printf("splitted.at(0): %s ....................\n", splitted.at(0).c_str());
		if(splitted.size() == 2)
		{
			std::string filename = splitted.at(0);
			int size = Util::parseInt(splitted.at(1).c_str());

			//printf("FileInfo::build() ... filename: [%s], fileSize: [%d]\n", filename, size);
			setFileName(filename);
			setFileSize(size);
		}
	}

	std::string serialize()
	{
		char sizeBuffer[WORKER_MAX_BUFFER];
		Util::intToString(getFileSize(), sizeBuffer);

		std::string msg = "";

		msg += getFileName();
		msg += " ";
		msg += sizeBuffer;
		//printf("FileInfo::serialize() ... %s\n", msg.c_str());
		return msg;
	}

	void setFileSize(int s){
		fileSize = s;
	}

	void setFileName(std::string n)
	{
		fileName = n;
	}

	int getFileSize()
	{
		return fileSize;
	}

	std::string getFileName()
	{
		return fileName;
	}
};
#endif
//worker do work
//like send files
//recv files
//execute various commands
class Worker : public Threadable{
	private:
		const char * command; 		//command to execute
		int socket; 				//socket fd
		sockaddr * remoteAddress; 	//the remote address of the request originator
		int bytesReceived;
		int bytesSent;
		Node * node;

		std::vector<int> commandIds;
		int activeCommandId;
		//bool serverWorker; //belongs to the server
		//void respond();
		

		void handleError(const char *);

	protected:
		//only allow Worker's to call terminate
		void finishwork(int status);

	public:
		//virtual function inherited from Threadable
		void execute();
		void rollback();
		//Worker's own pure virtual function which inheriting classes will use to do their thing
		virtual void doWork() = 0;

		//for inheriting classes to call, in which case the worker super class will take care of it
		void fallbackCommandHandle(const char *);

		Worker(const char * command, int fd, sockaddr * remoteAddr, Node * n):
			Threadable(),
			socket(fd), 
			remoteAddress(remoteAddr),
			command(command),
			node(n)
		//	serverWorker(isS)
			{
				bytesReceived = 0;
				bytesSent = 0;
				
			};
		
		Worker(int fd, sockaddr * remoteAddr, Node * n):
			Threadable(),
			socket(fd),
			remoteAddress(remoteAddr),
			command(NULL),
			node(n)
		//	serverWorker(isS)
			{
				bytesReceived = 0;
				bytesSent = 0;
			};

		void addCommandId(int);

		const char * getCommand() const;
		int getSocket() const;
		int getBytesSent() const;
		int getBytesReceived() const;
		//const void *getData() const;
		sockaddr * getRemoteAddress() const;
		Node * getNode() const;
		std::vector<int> getCommandIds() const;
		int getActiveCommandId() const;

		void setCommand(const char *);
		void setSocket(int);
		//void setData(const void *);
		void setRemoteAddress(sockaddr *);
		void setBytesSent(int);
		void setBytesReceived(int);
		void setNode(Node *);
		void setCommandIds(std::vector<int>);
		void setActiveCommandId(int);

		bool isCommand(const char *);
		bool isQuitCommand(const char *);
		bool isPutCommand(const char *);
		bool isGetCommand(const char *);
		bool isDeleteCommand(const char *);
		bool isCdCommand(const char *);
		bool isTerminateCommand(const char *);

		bool validFile(const char *);

		bool commandIsTerminated(int);

		void rollback(int);
		//bool isServer() const;
		//define special commands
		/*
		void put(const char *);
		void get(const char *, int);

		void sendFile(const char *);
		void recvFile(const char *);
		*/
		void del(char *);
		void cdr(const char *);
		void useBash(char *,  char *);
		void quit();

		void extractCommand(const char *, char *, int);
		void extractArgs(const char *, char *, int);

		static const std::string commands[];
 		static const std::string quit_command;
 		static const std::string put_command;
 		static const std::string get_command;
 		static const std::string delete_command;
		static const std::string cd_command;
		static const std::string terminate_command;

 		static char shell[];


};
	//you have to define the static const shit outside of the class def
	const std::string Worker::quit_command = "quit";
	const std::string Worker::commands[] = {"get", "put", "delete", "ls", "cd", "mkdir", "pwd", "quit", "terminate"};
	const std::string Worker::put_command = "put";
	const std::string Worker::get_command = "get";
	const std::string Worker::delete_command = "delete";
	const std::string Worker::cd_command = "cd";
	const std::string Worker::terminate_command = "terminate";

	char Worker::shell[] = "/bin/sh";
#endif