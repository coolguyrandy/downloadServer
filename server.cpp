#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdlib>
#include<cstdio>
#include<fcntl.h>
#include<cstdio>
#include<sys/stat.h>
#include<stdlib.h>
#include<sys/time.h>
#include<iostream>
#include<cstring>
#include"serverHeader.h"

int main(){

	int listenSocket, connectSocket;
	struct sockaddr_in server_address, client_address;
	socklen_t client_len = sizeof(client_address);
	char buffer[1024];

	if((listenSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1){
	perror("Listen socket creation failed");
	exit(EXIT_FAILURE);
	}
	


	//Configuring server address
	server_address.sin_family = PF_INET;//IPV4
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8000);

	int opt = 1;
	if( setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	if(bind(listenSocket, (struct sockaddr*)&server_address, sizeof(server_address)) <0 ){
		perror("Listen socket bind failed");
		close(listenSocket);
		exit(EXIT_FAILURE);
	}

	if(listen(listenSocket, 5) < 0){
		perror("Listen Failed");
		close(listenSocket);
		exit(EXIT_FAILURE);
	}
	printf("Server listening on port 8000\n");

	//Accept a connection
	if( (connectSocket = accept(listenSocket, (struct sockaddr *)&client_address, &client_len)) < 0){
		perror("Accept failed");
		close(listenSocket);
		exit(EXIT_FAILURE);
	}
	printf("Client connected\n");


	sendFile(connectSocket, "testfile.txt");
	
	
	close(connectSocket);
	close(listenSocket);

	return 0;
}
