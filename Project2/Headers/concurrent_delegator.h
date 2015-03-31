#ifndef CONCURRENT_DELEGATOR_H
#define CONCURRENT_DELEGATOR_H

#include "delegator.h"
#include "thread_controller.h"

#include <unistd.h>
#include <vector>
#include <pthread.h>

class ConcurrentDelegator : public Delegator {
	private:
		void handleError(const char *);

		ThreadController * threadController;

	public:
		ConcurrentDelegator();
		
		ThreadController * getThreadController() const;

		void setThreadController(ThreadController *);

		void handle(int, sockaddr *);

		void delegate(Worker *);
};
#endif