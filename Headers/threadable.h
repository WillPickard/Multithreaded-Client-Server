//threadable.h

/**
*	The point of this file is to define an interface
*	which will allow objects to extend and run as threads
**/

#ifndef THREADABLE_H
#define THREADABLE_H

#include <pthread.h>
#include <signal.h>//pthread_kill
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

static const int THREADABLE_WAKE_SIGNAL = SIGCONT; //defined in <signal.h>
static const int THREADABLE_SLEEP_SIGNAL = SIGUSR1; //signal to sleep on
static const int THREADABLE_FINISH_SIGNAL = SIGUSR2;
//static const int THREADABLE_ROLLBACK_SIGNAL = SIGALARM;

void * threadable_execution_routine(void *);
void threadable_wake_handler(int);
void threadable_sleep_handler(int);
void threadable_finish_handler(int);
void threadable_rollback_handler(int);

class Threadable 
{
	private:
		//the id of the thread. will be a process id
		pthread_t * id; 
		bool done;

		char * cwd;

	//	Semaphore * mutex;
	protected:
		//only allow other threads to update the Working directory
		void updateWorkingDirectory()
		{
			chdir(getCwd());
		}

		void yield()
		{
			pthread_yield();
		}

		void finish()
		{
			setDone(true);
			pthread_exit(0);
		}

		void nap()
		{
			pause();
		}

		void wake()
		{
			//?
		}

	public:
		//pure virtual function forcing the interface
		//after creating the thread, execute is called
		virtual void execute() = 0; 
		virtual void rollback() = 0;

		//init the current working directory
		Threadable()
		{
			char dir[FILENAME_MAX]; //FILENAME_MAX defined in stdio.h
			getcwd(dir, sizeof(dir));
			setCwd(dir);

		//	mutex = new Semaphore(0);
	
		};

		pthread_t run()
		{
			pthread_t * thread = (pthread_t *) malloc(sizeof(pthread_t *));
			setId(thread);
			pthread_create(thread, NULL, threadable_execution_routine, (void *) this);
			//printf("\tIn Threadable::run() ... thread is: %d\n", (*thread));
			return (*thread);
		}

		

		pthread_t * getId() const
		{
			return id;
		}

		bool isDone() const
		{
			return done;
		}

		const char * getCwd() const
		{
			return (const char *) cwd;
		}

		void setCwd(char * c)
		{
			cwd = c;
		}

		void setId(pthread_t * t)
		{
			id = t;
		}

		void setDone(bool d)
		{
			done = d;
		}
};

void * threadable_execution_routine(void * r)
{
	//first register the signal handler
	struct sigaction sleep_action, wake_action, finish_action;//, rollback_action;
	sleep_action.sa_handler = threadable_sleep_handler;
	wake_action.sa_handler = threadable_wake_handler;
	//finish_action.sa_handler = threadable_finish_handler;

	//install the actions
	sigaction(THREADABLE_SLEEP_SIGNAL, &sleep_action, NULL);
	sigaction(THREADABLE_WAKE_SIGNAL, &wake_action, NULL);
	sigaction(THREADABLE_FINISH_SIGNAL, &finish_action, NULL);
	//sigaction(THREADABLE_ROLLBACK_SIGNAL, &rollback_action, NULL);

	Threadable * runner = (Threadable *) r;
	//runner->yield(); //testing
	runner->execute();
	runner->setDone(true); 
	pthread_exit(0);
}

//respond to the sleep signal
void threadable_sleep_handler(int signo)
{
	if(signo == THREADABLE_SLEEP_SIGNAL)
	{
		pause();
	}
}

//respond to the wake signal
void threadable_wake_handler(int signo)
{
	if(signo == THREADABLE_WAKE_SIGNAL)
	{
		
	}
}

//respond to finish signal
void threadable_finish_handler(int signo)
{
	if(signo == THREADABLE_FINISH_SIGNAL)
	{
		pthread_exit(0);
	}
}
/*
//respond to rollback signal
void threadable_rollback_handler(int signo)
{
	if(signo == THREADABLE_ROLLBACK_SIGNAL)
	{
		//rollback();
	}
}
*/
#endif