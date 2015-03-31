//semaphores
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#ifndef PTHREAD_INCLUDED
#define PTHREAD_INCLUDED
#include <pthread.h>
#endif
/***
*	Provide a sync class for pthreads
*
***/
class Semaphore {
	private:
		int allowed; //number of allowed entries per 
		pthread_mutex_t m;
		pthread_cond_t c;

		void _lock();
		void _unlock();

	public:
		//constructor
		Semaphore(int);

		//decrement
		void p();
		//increment
		void v();
		//poll the blocked threads and allow one to go
		void signal();
		//wake all threads
		void broadcast();


};

#endif