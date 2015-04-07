//file_sender.cpp

#include "../Headers/file_sender.h"

//send the file
void FileSender::doWork()
{
	printf("FileSender::doWork() (%d) ... fileName: (%s)\n", (*getId()), getFileName());
	sendFile(getFileName());
	//finishwork(0);
}