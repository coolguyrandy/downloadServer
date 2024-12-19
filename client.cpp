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
#include"clientHeader.h"



int main(int argc, char*argv[]){
	char *ip = argv[1];
	int port = atoi(argv[2]);
	int socketClient;
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[1024];
	int err, n;

	if( (socketClient = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		perror("Socket creation failed");
		exit(EXIT_FAILURE);		
	} 

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);

	if(connect(socketClient, (struct sockaddr*)&addr, sizeof(addr))<0){
		perror("Failed to connect");
		exit(EXIT_FAILURE);
	}
	printf("Connected to the server\n");

	
	int fileFd = open("testReceipt", O_RDWR | O_CREAT, 0700);

	//Receive file
	recvFile(socketClient, fileFd);
	
	
	close(fileFd);
	close(socketClient);
	return 0;
}
