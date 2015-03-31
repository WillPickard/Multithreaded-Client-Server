#ifndef CONCURRENT_DELEGATOR_H
#define CONCURRENT_DELEGATOR_H

#include "delegator.h"
#include <pthread.h>
#include <vector>

class ConcurrentDelegator : public Delegator {
	private:
		std::vector<pthread_t> threads;
	public:
		std::vector<pthread_t> getThreads() const;
		void setThreads(std::vector<pthread_t>);

		void newThread(Worker *);


};
#endif