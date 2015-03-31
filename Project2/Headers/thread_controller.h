//thread_controller.h
#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

//#ifndef PTHREAD_INCLUDED
//#define PTHREAD_INCLUDED
#include <pthread.h>
//#endif

#include "threadable.h"
#include <exception>
//#include <vector>
#include <unordered_map>

class ThreadCreationException;
class ThreadJoinException;
class ThreadDoesNotExistException;

//threadController exception classes
//ThreadCreationException is thrown when there is an error in creating a new thread
class ThreadCreationException : public std::exception
{
	private:
		pthread_t * culprit;
	public:
		ThreadCreationException(pthread_t * thread):
			culprit(thread)
			{};

		pthread_t * getCulprit() const
		{
			return culprit;
		}
		
		virtual const char * what() const throw()
		{
			return "Error creating new thread";
		}
};

//exception to handle join
class ThreadJoinException : public std::exception
{
	private:
		pthread_t * culprit;
	public:
		ThreadJoinException(pthread_t * thread):
			culprit(thread)
			{};

		pthread_t * getCulprit() const
		{
			return culprit;
		}
		
		virtual const char * what() const throw()
		{
			return "Error joining with new thread";
		}
};

//exception to handle an attempt to access a non existant thread
class ThreadDoesNotExistException : public std::exception
{
	private:
		pthread_t * culprit;
	public:
		ThreadDoesNotExistException(pthread_t * thread):
			culprit(thread)
			{};

		pthread_t * getCulprit() const
		{
			return culprit;
		}
		
		virtual const char * what() const throw()
		{
			return "An attempt to access a thread that does not exist was made";
		}
};



class ThreadController {
	private:
		std::unordered_map<pthread_t , Threadable *> * threads;
	//	std::vector<Threadable *> * threads;
	public:	
		ThreadController(){
			//threads = new std::vector<Threadable * >();
			threads = new std::unordered_map<pthread_t , Threadable *> ();
		};
		//create a new thread
		pthread_t newThread(void * (*routine)(void *), void * param);
		pthread_t newThread(Threadable *);

		void * join(pthread_t thread);
		void * join(pthread_t thread, Threadable *);

		void signal(pthread_t, int);
		void euthanize(pthread_t);
		void kill(pthread_t);
		void killGracefully(pthread_t);
		void wake(pthread_t);

		std::unordered_map<pthread_t , Threadable *> * getThreads() const;
		//std::vector<Threadable *> * getThreads() const;

		void setThreads(std::unordered_map<pthread_t, Threadable *> *);
		//void setThreads(std::vector<Threadable *> *);
		
		void addThread(std::pair<pthread_t , Threadable *>);
		void addThread(pthread_t , Threadable *);
		pthread_t removeThread(pthread_t );

		//return the number of threads
		int numThreads();

		//find and return a threadable
		Threadable * find(pthread_t );

};

#endif