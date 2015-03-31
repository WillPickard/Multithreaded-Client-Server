//just make a huge file
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

const char * ALPHABET = "abcdefghijklmnopqrstuvwxyz !@#$^&*()";

int parseInt(const char *);
void fillBuf(char *, int);

int main(int argc, const char * argv[])
{
	cout << "huge file generator..." << endl;
	if(argc != 3)
	{
		cout << "./huge [filename] [size (bytes)] is how you call this" << endl;
		return 1;
	}

	srand(time(NULL));

	const char * filename = argv[1];
	int filesize = parseInt(argv[2]);
	int bytesWritten = 0;
	char buf[256];

	ofstream file (filename);

	while(bytesWritten < filesize)
	{
		memset(&buf, '\0', 256);
		fillBuf(buf, 256);
		file << buf;
		bytesWritten += 256;
	}

	file.close();

	cout << "done. wrote " << bytesWritten << " bytes" << endl;

	return 0;
}

int parseInt(const char * s)
{
	int number;
	std::istringstream ss(s);
	
	ss >> number;

	return number;
}

void fillBuf(char * buf, int len)
{
	int max = strlen(ALPHABET);

	for(int i = 0; i < len; i++)
	{
		buf[i] = 'b';//ALPHABET[rand() % max];
	}

}
