//server_runner.h

#ifndef SERVER_RUNNER_H
#define  SERVER_RUNNER_H

#include "server.h"
#include "threadable.h"

class ServerRunner : public Threadable
{
	private:
		Server * server;
	protected:
	public:
		//inherited from threadable
		void rollback();
		
		ServerRunner(Server *);

		void execute();

		void setServer(Server *);

		Server * getServer() const;

};

#endif