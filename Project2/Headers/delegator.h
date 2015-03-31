#ifndef DELEGATOR_H
#define DELEGATOR_H

//#include "../Headers/server.h"

#include "../Headers/worker.h"
#include <vector>

//class Server;

class Delegator {
	private:
		std::vector<Worker *> workers;
		
	public:
		virtual void handle(int, sockaddr *) = 0;

		void delegate(Worker *);
	

		std::vector<Worker *> getWorkers() const
		{
			return workers;
		}

		void addWorker(Worker *w)
		{
			workers.push_back(w);
		}


};
#endif