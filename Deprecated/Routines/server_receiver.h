//server_receiver_routine

#ifndef SERVER_RECEIVER_ROUTINE_H
#define SERVER_RECEIVER_ROUTINE_H

#include "../Classes/worker.cpp"
/***************
*
*	This class will encapsulate 
*	the worker class
*
*****************/
class WorkerBox {
	public: 
		 Worker * worker;

		 WorkerBox(Worker * worker):
		 	worker(worker)
		 	{};

		 Worker * getWorker() const
			{
				return worker;
			}	
};

/***
*
*	workerRoutine is the routine a thread will execute to receive commands from the client and execute them
*	The void pointer is an object of type WorkerBox
*	To retreive the worker from the box just cast
*	it to a WorkerBox * then call ->getWorker()
*
****/
void * serverReceiverRoutine(void *b)
{
	//cast it out of the void pointer 
	WorkerBox * box = (WorkerBox *) b;
	Worker * worker = box->getWorker();

	char buf[512];
	int bytesReceived = 0;
	//we have to wait for the command 
	do
	{
		//memset it out because the internet says to
		memset(buf, 0, sizeof(buf));
		bytesReceived = recv(worker->getFileDescriptor(), buf, sizeof(buf), 0);

		const char * command = worker->extractCommand(buf);
		const char * args = worker->extractArgs(buf);
		//printf("sizeof buf: %d\n", sizeof(buf));
		//printf("sizeof &buf: %d\n", sizeof(&buf));
		//printf("serverReceiverRoutine received: %s\n", buf);
		if(worker->isGetCommand(command))
		{
			//if the server encounters a get command, then it has to put the file to the client
			//char newCommand[256] = "put ";
			//strcat(newCommand, args);
			//worker->setCommand((const char *)newCommand);
			worker->setCommand((const char *) buf);
			worker->sendFile(args);
		}
		else
		{
			worker->setCommand((const char *) buf);
			worker->execute();

			if(bytesReceived <= 0)
			{
				//handleError("recv");
				//printf("\tbytesreceived <= 0...\n");
				//handleError will exit but whatevs
				//pthread_exit(0);
			}
		}

		//printf("\tReceived %s... creating worker...\n", buf);
	} while(!worker->isDone());

	close(worker->getFileDescriptor());
	//exit without error
	//printf("Worker is done in its workerRoutine()\n");
	pthread_exit(0);
}

#endif