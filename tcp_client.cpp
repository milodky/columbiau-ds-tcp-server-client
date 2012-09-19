#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

// Constants 
#define BUFFER_SIZE 254

// Utility functions
int readLine(int sockfd, void * buffer, size_t length); // reads a line off of a stream socket
void fatalPError(const char* msg); // prints out errno friendly message and exits
void fatalError(const char* msg); // prints out an error message and exits
void usage();

int main(int argc, char *argv[])
{


    // Validate & read arguments
	char* server_addr_str;
    int server_port;
    char* file_name;	
	char* local_dir;
    struct stat st;

    if (argc < 5)
        usage();

    server_addr_str = argv[1];

    if ( (server_port = atoi(argv[2])) <= 0)
    {
        printf("Invalid server port specified\n");
        usage();
    }

    file_name = argv[3];

    local_dir = argv[4];
    if (stat(local_dir, &st) != 0)
    {
        printf("Unable to access specified directory %s\n", local_dir);
        perror("Error");
        usage();        
    }

    // Src/Dst Addresses structures
    struct in_addr server_addr;
    struct sockaddr_in server_sockaddr;

    // Convert server address to binary representation
    memset(&server_addr, 0, sizeof(struct in_addr));
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, server_addr_str, &server_addr);
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr = server_addr;
    server_sockaddr.sin_port = htons(server_port);
    
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    	fatalPError("socket");

    // Connect to server
    if ( (connect(sockfd, (struct sockaddr*)&server_sockaddr, sizeof(struct sockaddr_in))) < 0)
    	fatalPError("connect");

    // Send file request message "REQ FILENAME"
    char msg[BUFFER_SIZE];
    int out_size = sprintf(msg, "REQ %s\n", file_name);
    if (out_size < 0)
    	fatalPError("sprintf");

    int wrote;
    if ( (wrote = write(sockfd, msg, out_size)) < 0)
    	fatalPError("write");


    // Read server's response
    int ret = readLine(sockfd, msg, BUFFER_SIZE);
    if (ret < 0)
    {
        printf("Error reading from connection..");
        fatalPError("read");
    }

    
    // Did we receive an error from the server ? 
    if (strcmp(msg, "ERROR") == 0) 
    {
    	// Server reported error
    	memset(msg, '\0', BUFFER_SIZE);
    	sprintf(msg, "File %s does not exist in the server\n", file_name);
    	fatalError(msg);
    }
	else if (strcmp(msg, "ERRORSIZE") == 0)
    {
    	// Server reported file size error
    	memset(msg, '\0', BUFFER_SIZE);
    	sprintf(msg, "Requested file %s is too big\n", file_name);
    	fatalError(msg);
    }
    else
    {

        // No errors
    	// Tokenize
        char* token = strtok(msg, " ");
        
        // Validate client message structure
        if (strcmp(token, "FILESIZE") != 0)
        {
            printf("Server has sent a malformed response. Closing connection.\n");
            close(sockfd);
            exit(1);
        }

        // Extract requested file size
        char* file_size_str = strtok(NULL, " ");
        size_t file_size = atol(file_size_str);

        // Read file data from stream
		void* file_buffer =  malloc(file_size);
		bzero(file_buffer, file_size);
		if (file_buffer == NULL)
			fatalPError("malloc");
		
		size_t totalRead = 0;
		char* buff_pointer = (char*)file_buffer;
		while (totalRead < file_size)
		{
        	int bytesRead = read(sockfd, buff_pointer, 1024*1024);
        	if (bytesRead < 0)
        	{
        		fatalError("Reached end of stream before file was received completely");
        		break;
        	}

        	totalRead += bytesRead;
        	buff_pointer += bytesRead;

        }

        // Construct destination full path
		char* full_path = (char*)malloc(strlen(local_dir) + strlen(file_name) + 2);
		if (full_path == NULL)
			fatalPError("malloc");
		
		strcpy(full_path, local_dir);
		strcat(full_path, file_name);


        // Save file
		FILE *file;
		file = fopen(full_path, "w"); 
        if (file == NULL)
            fatalPError("fopen");
        
		fwrite(file_buffer, totalRead, 1, file);
		fclose(file); 
		
		free(file_buffer);
		free(full_path);

        // Print message & exit
        printf("%s saved\n", file_name);

    }

    return 0; 
}



void usage()
{
    printf("Usage: tcp_client server_host server_port file_name dst_directory\n");
    exit(0);
}
