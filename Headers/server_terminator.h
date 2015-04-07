//server_terminator.h

#ifndef SERVER_TERMINATOR_H
#define SERVER_TERMINATOR_H

#include "server.h"
#include <exception>



class ServerTerminator : public Server 
{
	private:
		Server * slave;

	public:
		//port, domain, type, protocol
		ServerTerminator(int port, int sdomain, int stype, int sprot, Server * server):
			Server(port, sdomain, stype, sprot),
			slave(server)
			{};
	

		Server * getSlave() const;

		void setSlave(Server *);
		
		//everything else in Server we want to keep the same, except for the wait function, that we want to switch up
		void wait();

		void run();

		
};
#endif