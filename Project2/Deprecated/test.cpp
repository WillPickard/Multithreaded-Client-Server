#include <unistd.h>

int main(int argc, const char * argv[])
{
	execl("/bin/sh",  "/bin/sh", "-c", "ls", (char *) NULL);
}