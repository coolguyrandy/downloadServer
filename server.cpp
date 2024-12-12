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


int readn(int socket, char *ptr, size_t n){
	size_t totalBytesRead = 0;

	while(totalBytesRead < n){
		size_t bytesRead = read(socket, ptr+totalBytesRead, n-totalBytesRead);
		if(bytesRead < 0){
			perror("Byte receipt error");
			exit(EXIT_FAILURE);
		}
		else if(bytesRead == 0){
			break;
		}
		totalBytesRead += bytesRead;
	}

	return totalBytesRead;
}


int writen(int socket, char *ptr, size_t n){
	size_t totalBytesWritten = 0;

	while(totalBytesWritten < n){
		size_t bytesWritten = write(socket, ptr+totalBytesWritten, n-totalBytesWritten);
		if(bytesWritten < 0){
			perror("Byte write error");
			exit(EXIT_FAILURE);
		}
		else if(bytesWritten == 0){
			break;
		}
		totalBytesWritten += bytesWritten;
	}

	return totalBytesWritten;
}

long getFileSize(int fd){
	struct stat stat_buf;
	int rc = fstat(fd, &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;	
}


int fileWrite(int readFd, int writeFd, int readFileSize, size_t bufSize){

	char *buf = (char *) malloc(bufSize);//dynamically allocating character buffer with specified file size
	
	int bytesWritten = 0;
	int bytesLeft = readFileSize;
	while(bytesWritten < readFileSize){
		if(bytesLeft < bufSize){//special case when bytes left to be written is less than buffer size. Must adjust number of bytes to be read from file to prefvent reading invalid data
			readn(readFd, buf, bytesLeft);
			writen(writeFd, buf, bytesLeft);
			bytesWritten += bytesLeft;
		}
		else{
			readn(readFd, buf, bufSize);
			bytesWritten += writen(writeFd, buf, bufSize);
			bytesLeft -= bufSize;
		}
	}

	free(buf);

	return 0;
}

int sendFile(int socketFd, char *filename){
	int fileFd;
	if( (fileFd = open(filename, O_RDONLY) ) < 0){
		perror("File open failed");
		exit(EXIT_FAILURE);
	}


	//send file size to client
	size_t fileSize = getFileSize(fileFd);
	char fileSizeMsg[8];
	memset(fileSizeMsg, 'x', sizeof(fileSizeMsg));
	memcpy(fileSizeMsg, &fileSize, sizeof(size_t));
	writen(socketFd, fileSizeMsg, 8);

	fileWrite(fileFd, socketFd, fileSize, 128);

	return 0;
}
	

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


	sendFile(connectSocket, "testFile");
	
	
	close(connectSocket);
	close(listenSocket);

	return 0;
}
