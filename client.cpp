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
	size_t nleft;
	ssize_t nread;
	nleft = n;
	while(nleft > 0){
		if((nread = read(socket, ptr, nleft)) < 0){
			if(nleft == n)
				return -1;
			else
				break;		
		}
		else if(nread == 0)
			break;
		
		nleft -=nread;
		ptr += nread;
	}

	return (n-nleft);
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

size_t recvFileSize(int socket){
	//Receive initial message (FILESIZE)
	char fileSizeMsg[8];
	memset(fileSizeMsg, 'x', 8);
	
	int bytesRead;
	if( (bytesRead = readn(socket, fileSizeMsg, 8)) < 0){
		perror("Error reading from socket");
		return -1;
	}

	size_t fileSize;
	memcpy(&fileSize, fileSizeMsg, sizeof(size_t));
	return fileSize;
}


int fileWrite(int readFd, int writeFd, int readFileSize, size_t bufSize){

	char *buf = (char *) malloc(bufSize);//dynamically allocating character buffer with specified file size
	
	int bytesWritten = 0;
	int bytesLeft = readFileSize;
	while(bytesWritten < readFileSize){
		if(bytesLeft < bufSize){//special case when bytes left to be written is less than buffer size. Must adjust number of bytes to be read from file to prevent reading invalid data
			readn(readFd, buf, bytesLeft);
			writen(writeFd, buf, bytesLeft);
			bytesWritten += bytesLeft;
		}
		else{
			readn(readFd, buf, bufSize);
			bytesWritten += writen(writeFd, buf, bufSize);
			bytesLeft -= bufSize;
		}

		printf("Received %d bytes out of %d total \r", bytesWritten, readFileSize);
	}
	printf("\nDownload complete.\n");
	free(buf);

	return 0;
}




int recvFile(int socket, int fileFd){

	size_t fileSize = recvFileSize(socket);

	fileWrite(socket, fileFd, fileSize, 128);

	return 0;
}


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
