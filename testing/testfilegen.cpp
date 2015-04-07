//just make a huge file
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

int numlines = 10000000;
const char * ALPHABET = "abcdefghijklmnopqrstuvwxyz !@#$^&*()";

void fillBuf(char *, int);

int main(int argc, const char * argv[])
{
	cout << "test file generator..." << endl;

	srand(time(NULL));
	
	
	char buf[256];
	

	/*
	string filename = "start.txt";
	ofstream file (filename.c_str());

	int i = 0;
	for(int i; i < numlines; i++)
	{
		memset(&buf, '\0', 256);
		sprintf(buf, "%d", i);
		strcat(buf, "\n");
		file << buf;
	}

	file.close();
	*/


	int bw = 0;
	string filename1 = "test.txt";
	ofstream file1 (filename1.c_str());
	int i = numlines;

//	while(bytesWritten < filesize)
//	{
		memset(&buf, '\0', 256);
		fillBuf(buf, 256);
		file1 << buf;
		bw += 256;
//	}
		memset(&buf, '\0', 256);
		fillBuf(buf, 256);
		file1 << buf;
		bw += 256;
		file1 << '!';

	file1.close();

	cout << "done" << endl;

	return 0;
}

void fillBuf(char * buf, int len)
{
	int max = strlen(ALPHABET);

	for(int i = 0; i < len; i++)
	{
		buf[i] = ALPHABET[rand() % max];
	}

}
