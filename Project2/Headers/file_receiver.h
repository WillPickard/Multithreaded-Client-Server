//file_receiver.h
#ifndef FILE_RECEIVER_H
#define FILE_RECEIVER_H

#include "receiver.h"

class FileReceiver : public Receiver
{
	private:
		const char * fileName;

	public:
		//the virtual method inherited from worker
		FileReceiver(const char * command, int socket, struct sockaddr * remoteAddr, Node * n, const char * filename):
			Receiver(command, socket, remoteAddr, n)
			//fileName(filename)
			{
				setFileName(filename);
			};

		FileReceiver(int socket, struct sockaddr * remoteAddr, Node * n, const char * filename):
			Receiver(socket, remoteAddr, n)
			//fileName(filename)
			{
				setFileName(filename);
			};

		FileReceiver(int socket, struct sockaddr * remoteAddr, Node * n):
			Receiver(socket, remoteAddr, n)
			{
			//	setFileName(filename);
			};


		const char * getFileName() const
		{
			return fileName;
		}

		void setFileName(const char * fn)
		{
			char * t = (char *) malloc(strlen(fn));
			strcpy(t, fn);
			fileName = t;
		}

		void doWork();
};

#endif