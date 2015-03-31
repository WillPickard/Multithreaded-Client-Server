//server_runner.cpp

#include "../Headers/server_runner.h"

ServerRunner::ServerRunner(Server * s):
	Threadable(),
	server(s)
	{};


void ServerRunner::setServer(Server * s)
{
	server = s;
}

Server * ServerRunner::getServer() const
{
	return server;
}


//just run it
void ServerRunner::execute()
{
	getServer()->run();
}

void ServerRunner::rollback()
{

}