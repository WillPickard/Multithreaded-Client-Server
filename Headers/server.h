#ifndef SERVER_H
#define SERVER_H

/****
*
*	@file server.h
*
*	Define the server object
*	Client must implement:
*		1. get (get <remote_filename>) -- Copy file with the name <remote_filename> from
			remote directory to local directory.
		2. put (put <local_filename>) -- Copy file with the name <local_filename> from local
			directory to remote directory.
		3. delete (delete <remote_filename>) – Delete the file with the name <remote_filename>
			from the remote directory.
		4. ls (ls) -- List the files and subdirectories in the remote directory.
		5. cd (cd <remote_direcotry_name> or cd ..) – Change to the <remote_direcotry_name >
			on the remote machine or change to the parent directory of the current directory
		6. mkdir (mkdir <remote_directory_name>) – Create directory named
			<remote_direcotry_name> as the sub-directory of the current working directory on the
			remote machine.
		7. pwd (pwd) – Print the current working directory on the remote machine.
		8. quit (quit) – End the FTP session.

*	Constructor takes a port number where server executes.  
*
******/

#include <stdio.h>
#include <string>
#include <unordered_map>
#include "node.h"
#include "concurrent_delegator.h"
#include "semaphore.h"

class Server : public Node {
	
	private:
		struct sockaddr_in serverAddress;
		struct sockaddr_in terminateAddress;

		ConcurrentDelegator * delegator;	

		int terminateSocket;

		void createBind();
		void wait();

		void delegate(int socketfd, sockaddr * remoteAddr);

		Semaphore * fileMapMutex;
		std::unordered_map<int, Semaphore *> * fileMap;

		int hashFileName(const char *);

		Semaphore * getFileMapMutex() const;
		void setFileMapMutex(Semaphore *);

	public:	
		static const int backlog = 5;


		//constructor with no terminate prot
		Server(int, int, int, int);
		//consructor with terminate port
		Server(int, int, int, int, int);

		sockaddr_in getServerAddress() const;
		sockaddr_in getTerminateAddress() const;
		ConcurrentDelegator * getDelegator() const;
		int getTerminateSocket() const;
		std::unordered_map<int, Semaphore *> * getFileMap() const;

		void setServerAddress(struct sockaddr_in);
		void setRemoteAddress(struct sockaddr_in);
		void setTerminateAddress(struct sockaddr_in);
		void setDelegator(ConcurrentDelegator *);
		void setTerminateSocket(int);
		void setFileMap(std::unordered_map<int, Semaphore *> *);


		void requestAccess(const char *);
		void relenquishAccess(const char *);
		//static void run(int, int, int, int);
		void run();
		void threadCheckout(Threadable *);
		void closeConnection(int);
		void setUp();

};


#endif