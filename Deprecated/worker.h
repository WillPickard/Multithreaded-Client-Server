#ifndef WORKER_H
#define WORKER_H

/*
*
*

DEPRECATED ON 01/30/2015 to add threadable interface 

*
*
*
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

class Worker {
	private:
		const char * command; 	//command to execute
		int fileDescriptor; 	//socket fd
		sockaddr * remoteAddress; //the remote address of the request originator
		//const void *data; 		//data to send
		int totalBytesReceived;
		int totalBytesSent;

		int id;

		bool done;

		//void respond();
		void terminate(int status);

		void handleError(const char *);

		
		
	public:
		Worker(const char * command, int fd, sockaddr * remoteAddr):
			fileDescriptor(fd), 
			remoteAddress(remoteAddr),
			command(command)
			{
				totalBytesReceived = 0;
				totalBytesSent = 0;
				
			};
		
		Worker(int fd, sockaddr * remoteAddr):
			fileDescriptor(fd),
			remoteAddress(remoteAddr),
			command(NULL)
			{
				totalBytesReceived = 0;
				totalBytesSent = 0;
			};

		void * execute();

		const char * getCommand() const;
		int getFileDescriptor() const;
		int getTotalBytesSent() const;
		int getTotalBytesReceived() const;
		int getId() const;
		//const void *getData() const;
		sockaddr * getRemoteAddress() const;

		bool isDone() const;

		void setCommand(const char *);
		void setFileDescriptor(int);
		//void setData(const void *);
		void setRemoteAddress(sockaddr *);
		void setId(int);

		void setTotalBytesSent(int);
		void setTotalBytesReceived(int);

		bool isCommand(const char *);
		bool isQuitCommand(const char *);
		bool isPutCommand(const char *);
		bool isGetCommand(const char *);
		bool isDeleteCommand(const char *);
		bool isCdCommand(const char *);

		//define special commands
		void put(const char *);
		void get(const char *, int);
		void del(const char *);
		void cdr(const char *);
		void useBash(const char *);
		void quit();

		void sendFile(const char *);
		void recvFile(const char *);

		const char * extractCommand(const char *);
		const char * extractArgs(const char *);

		static const std::string commands[];
 		static const std::string quit_command;
 		static const std::string put_command;
 		static const std::string get_command;
 		static const std::string delete_command;
		static const std::string cd_command;

 		static const char * shell;


};
	const std::string Worker::quit_command = "quit";
	const std::string Worker::commands[] = {"get", "put", "delete", "ls", "cd", "mkdir", "pwd", "quit"};
	const std::string Worker::put_command = "put";
	const std::string Worker::get_command = "get";
	const std::string Worker::delete_command = "delete";
	const std::string Worker::cd_command = "cd";

	const char *  Worker::shell = "/bin/sh";
#endif