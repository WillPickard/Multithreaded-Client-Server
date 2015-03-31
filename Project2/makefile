all: server client

server: Classes/server.cpp myftpserver.cpp
	g++ -std=c++11 -o myftpserver myftpserver.cpp -lpthread

client: Classes/client.cpp myftp.cpp
	g++ -std=c++11 -o myftp myftp.cpp -lpthread

run: all
	./myftpserver 2234

run_server: server
	./myftpserver 2234 2235
	
run_client: client
	./myftp localhost 2234 2235

test: test.cpp
	g++ -o test test.cpp
	./test

