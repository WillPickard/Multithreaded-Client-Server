#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "my_echo_socket"

int main(void){
  int sid, len, t;	
	struct sockaddr_un remote;
	char buf[256];

	if((sid=socket(AF_UNIX, SOCK_STREAM, 0))==-1){
		perror("socket");
		exit(1);
	}

	printf("trying to connect...\n");

	remote.sun_family=AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path)+sizeof(remote.sun_family);
	if(connect(sid, (struct sockaddr *)&remote, len)==-1){
		perror("connect");
		exit(1);
	}

	printf("connected...\n");

  while(printf("> "), read(0,buf, 256)){
    if(send(sid, buf, strlen(buf), 0)==-1){
			perror("Send");
			exit(1);
		}
		
		if((t=recv(sid,buf, 256, 0))>0){
			buf[t]='\0';
			printf("The server thinks you said: %s",buf);
		}
		else{
      if(t<0){perror("recv"); exit(1);}
			else{
				printf("Server connection closed\n");
				exit(1);
			}
		}
	}
close(sid);
return 0;
}
