//client_routines.h
#ifndef CLIENT_ROUTINES_H
#define CLIENT_ROUTINES_H

#include "../Headers/client.h"
/***
*
*	Transmission routine and encapsulation
*
*	Transmissions occur when we want to send a file or something
**
class ClientBox {
	public:
		Client * client;

		ClientBox(Client * c):
			client(c)
			{};

		Client * getClient() const 
		{
			return client;
		}
};


void * transmitRoutine(void * cb)
{
	ClientBox * box = (ClientBox *)cb;
	Client * client = box->getClient(); 
	char buf[256];

	while(printf("myftp> "), std::cin.getline(buf, sizeof(buf)))
		{
			//buf[sizeof(buf)] = "\0";			
			if(send(client->getSocket(), buf, sizeof(buf), 0) == -1)
			{
				//handleError("send");
			}

			//totalBytesSent += sizeof(buf);
		}
}


/***
*
*	Receive routine 
*
***
void * receiveRoutine(void *cb)
{
	ClientBox * box = (ClientBox *)cb;
	Client client = box->getClient(); 
}
/**/


#endif