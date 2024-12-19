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
	

