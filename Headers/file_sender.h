//file_sender.h

#ifndef FILE_SENDER_H
#define FILE_SENDER_H

#include "sender.h"

class FileSender : public Sender
{
	private:
		const char * fileName;

	public:
		//the virtual method inherited from worker
		FileSender(const char * command, int socket, struct sockaddr * remoteAddr, Node * n, const char * filename):
			Sender(command, socket, remoteAddr, n)
			//fileName(filename)
			{
				setFileName(filename);
			};

		FileSender(int socket, struct sockaddr * remoteAddr, Node * n, const char * filename):
			Sender(socket, remoteAddr, n)
		//	fileName(filename)
			{
				setFileName(filename);
			};

		FileSender(int socket, struct sockaddr * remoteAddr, Node * n):
			Sender(socket, remoteAddr, n)
			{};

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