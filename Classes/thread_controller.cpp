//thread_controller.cpp

#include "../Headers/thread_controller.h"

std::unordered_map<pthread_t, Threadable *> * ThreadController::getThreads() const 
//std::vector<Threadable *> * ThreadController::getThreads() const
{
	return threads;
}

void ThreadController::setThreads(std::unordered_map<pthread_t, Threadable *> * t)
//void ThreadController::setThreads(std::vector<Threadable *> * t)
{
	threads = t;
}

void ThreadController::addThread(std::pair<pthread_t, Threadable *> pair)
{
	getThreads()->emplace(pair);
}

void ThreadController::addThread(pthread_t t, Threadable * threadable)
{
	
	std::pair<pthread_t, Threadable *> pair (t, threadable);
	addThread(pair);

}

pthread_t ThreadController::removeThread(pthread_t t)
{
	int numRemoved = getThreads()->erase(t);
	
	if(numRemoved == 0)
	{
		throw new ThreadDoesNotExistException(&t);
	}

	return t;
}

//also you can pass it an object of type threadable
pthread_t ThreadController::newThread(Threadable * runner)
{
	//all we have to do to the runner is call run
	pthread_t thread = runner->run();
	//printf("\tThreadController::newThread() ... pthread_t returned by runner->run(): %d\n", thread);
	addThread(thread, runner);
	//printf("\tThreadController::newThread() ... succesfully added thread\n");
	return thread;
}

/***
*
*	@func join
*		- wraps pthread_join(pthread_t thread, void * status)
*		- do not allow the current thread to execute until thread has finished
*		- This means that ThreadController::join() will not return until the thread has finished or an exception is raised
***/
void * ThreadController::join(pthread_t thread)
{
	void * status; //status will be filled in with the value returned by pthread_exit(int) where thread calls it
	int result; 

	result = pthread_join(thread, &status);
	//result will be 0 if it is all good, 1 otherwise
	if(result != 0)
	{
		throw new ThreadJoinException(&thread);
	}

	return status;
}

//overloaded 
void * ThreadController::join(pthread_t t, Threadable * thread)
{

}

//send a specific thread a specific singal
void ThreadController::signal(pthread_t thread, int signo)
{
	pthread_kill(thread, signo);
}
//send a sleep signal to the thread
void ThreadController::euthanize(pthread_t thread)
{	
	signal(thread, THREADABLE_SLEEP_SIGNAL);
}

//wake a thread from its slumber
void ThreadController::wake(pthread_t thread)
{
	signal(thread, THREADABLE_WAKE_SIGNAL);
}

//kill the thread
void ThreadController::kill(pthread_t thread)
{
	signal(thread, SIGKILL);
}

void ThreadController::killGracefully(pthread_t thread)
{
	signal(thread, THREADABLE_FINISH_SIGNAL);
}
//return the number of threads
int ThreadController::numThreads()
{
	return getThreads()->size();
}

Threadable * ThreadController::find(pthread_t thread)
{
	std::unordered_map<pthread_t , Threadable *>::const_iterator result = getThreads()->find(thread);
/*
	for(int i = 0; i < getThreads()->size(); i++)
	{
		if(thread == (*getThreads()->at(i)->getId()))
		{
			return getThreads()->at(i);
		}
	}
*/

	if(result == getThreads()->end())
	{
		return NULL;
	}

	//result->second will be the threadable object
	return result->second;

//	return NULL;
}
