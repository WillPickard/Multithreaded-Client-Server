#include "../Headers/semaphore.h"

Semaphore::Semaphore(int allowed):
	allowed(allowed)
	{
		pthread_cond_init(&c, NULL);
		pthread_mutex_init(&m, NULL);
	};

/*
*	Atomic decrement
*	
*/
void Semaphore::p()
{
	_lock();
	//don't ever let allowed get to 0
	while(allowed <= 0)
	{
		pthread_cond_wait(&c, &m);
	}

	allowed--;
	_unlock();
}

/*
*	Atomic increment
*	
*/
void Semaphore::v()
{
	_lock();
	allowed++;
	if(allowed > 0)
	{
		//signal 
		signal();
	}
	_unlock();
}

void Semaphore::broadcast()
{
	pthread_cond_broadcast(&c);
}

void Semaphore::signal()
{
	pthread_cond_signal(&c);
}

void Semaphore::_lock()
{
	pthread_mutex_lock(&m);
}

void Semaphore::_unlock()
{
	pthread_mutex_unlock(&m);
}