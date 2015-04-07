//channel.h
//define a class to allow two Workers or Nodes to communicate with one another

#ifndef CHANNEL_H
#define CHANNEL_H

#include "node.h"
#include "threadable.h"

class Channel 
{
	private:
		struct * addrinfo remoteAddress;
		bool open;
	public:
		//form channel between two nodes
		//using a socket
		Channel(Node *, Node *);

		//for a channel for communication between two threads
		Channel(Threadable *, Threadable *);

		struct * addrinfo getRemoteAddress() const;

		void setRemoteAddress(struct * addrinfo);

		//send data over the channel
		void send(void *);

		//recv data on the channel
		void recv(void *);

		//close the channel
		void close();

		//open the channel
		void open();

		void setOpen();

		bool getOpen() const;
};
#endif