//terminate_receiver.cpp

#include "../Headers/terminate_receiver.h"

void TerminateReceiver::doWork()
{
	printf("TerminateReceiver::doWork() (%d) \n", (*getId()));

	Node * node = getNode();
	char received[WORKER_MAX_BUFFER];
	//we will wait for a command and then extract the command id from it
	//upon receiving the ocmmand id, notify the server that the command needs to be terminated
	while(!isDone())
	{
		memset(&received, '\0', sizeof(received));
		receive(received, sizeof(received));

		//received is now a command id
		//convert the string to a char *
		int commandId = Util::parseInt(received);

		printf("TerminateReceiver::doWork() (%d) ... command Id: [%d]\n", (*getId()), commandId);

		//mark the command id for completion
		bool result = node->markCommand(commandId);

		if(result)
		{
			printf("TerminateReceiver::doWork() (%d) ... successfully terminated command [%d]\n", (*getId()), commandId);
		}
		else 
		{
			printf("TerminateReceiver::doWork() (%d) ...  command [%d] does not exist\n", (*getId()), commandId);
		}
	}
}

void TerminateReceiver::terminateCommand(int commandId)
{

}