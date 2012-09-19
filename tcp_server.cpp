#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "cache.h"

// Constants 
#define BUFFER_SIZE     254
#define BACKLOG         10
#define MAX_FILE_SIZE   30 * 1024 * 1024

// Utility functions
void usage();

int main(int argc, char *argv[])
{


    // Validate & read arguments
    int listen_port;
    char* file_directory;
    struct stat st;

    if (argc < 3)
        usage();

    if ( (listen_port = atoi(argv[1])) <= 0)
    {
        printf("Invalid port specified\n");
        usage();
    }

    file_directory = argv[2];
    if (stat(file_directory, &st) != 0)
    {
        perror("Unable to access specified directory");
        usage();        
    }


    // Buffer for incoming messages
    char buffer[BUFFER_SIZE];

    // Src/Dst Addresses structures
    struct in_addr listen_addr;
    struct sockaddr_in listen_sockaddr, client_addr;
    char client_addr_str[INET_ADDRSTRLEN];
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    // Initialize listen address structs
    memset(&listen_addr, 0, sizeof(in_addr));
    listen_addr.s_addr = htonl(INADDR_ANY);

    memset(&listen_sockaddr, 0, sizeof(struct sockaddr_in));
    listen_sockaddr.sin_family   = AF_INET;
    listen_sockaddr.sin_addr     = listen_addr;
    listen_sockaddr.sin_port     = htons(listen_port);


    // Create & bind socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        fatalPError("socket");

    if (bind(sockfd, (sockaddr*)&listen_sockaddr, sizeof(struct sockaddr_in)) == -1)
        fatalPError("bind");


    // Listen 
    if (listen(sockfd, BACKLOG) < 0)
        fatalPError("listen");



    // ----------------------------------------- //
    // Server is up and running
    // ----------------------------------------- //

    // Declarations
    struct cache cache;

    
    while (true)
    {

        // -------------------------------------------
        // Per connection flags
        bool cached = true; // indicates whether requested file was in cache
        bool no_errors = true; // indicates whether file was read successfully from disk


        // -------------------------------------------
        // Accept incoming connections (one at a time)
        int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);

        // Incoming connection, translate to human readable representation
        memset(&client_addr_str, '\0', INET_ADDRSTRLEN);
        if (inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, INET_ADDRSTRLEN) == NULL)
            strncpy(client_addr_str, "Unknown", 16);
        

        // --------------------
        // Connection Accepted: 
        // 1 - Expecting "REQ XXXX" message        

        // Receive and parse incoming message from client
        int ret = readLine(client_sockfd, buffer, BUFFER_SIZE);
        if (ret < 0)
        {
            perror("read");
            printf("Error reading from client. Closing connection..\n");
            close(client_sockfd);
            continue;
        }

        char* token = strtok(buffer, " ");
        
        // Validate client message structure
        if (strcmp(token, "REQ") != 0)
        {
            printf("Client %s has sent a malformed request. Closing connection.\n", client_addr_str);
            close(client_sockfd);
            continue;
        }

        // Extract requested file name
        char* requested_file = strtok(NULL, " ");
        printf("Client %s is requesting file %s\n", client_addr_str, requested_file);

        // Full path to file
        char file_path[255];        
        char file_name[255];
        strcpy(file_name, requested_file);
        strcpy(file_path, file_directory);
        strcat(file_path, "/");
        strcat(file_path, requested_file);


        // -------------------------------------------------
        // 2- Attempt to retrieve requested file from cache
        const struct cache_entry* cached_file;
        void* file_buffer;        
        long file_size;        
        cached_file = getCachedFile(&cache, file_path);

        if (cached_file == NULL)
        {
            // File is not in cache
            cached = false;

            // Read in requested file
            FILE *f;
            
            f = fopen(file_path, "rb");
            if (f)
            {
                // Determine file size:
                fseek (f , 0 , SEEK_END);
                file_size = ftell(f);
                rewind (f);

                // Validate requested file size is acceptable 
                if (file_size > MAX_FILE_SIZE)
                {
                    printf("Requested file is too big\n");
                    write(client_sockfd, "ERRORSIZE\n", 10);
                    no_errors = false;
                }

                // Begin reading file
                file_buffer = malloc(sizeof(char)*file_size+1);
                if (file_buffer == NULL)
                    fatalPError("malloc");

                if (readFile(f, file_buffer) < 0)
                {
                    free(file_buffer);
                    no_errors = false;
                }

            }
            else
            {
                perror("fopen");            
                printf("Error opening requesed file: %s\n", file_path);        
                write(client_sockfd, "ERROR\n", 6);
                no_errors = false;
            }
            fclose(f);

            // Cache file
            if (no_errors)
            {
                cached_file = cacheFile(&cache, std::string(file_path), file_buffer);

                if (cached_file == NULL) // file has successfully been cached, we'll used
                                        // cached entry from now on to avoid redundancy 
                    free(file_buffer);
            }


        }


        // -------------------------------------------------
        // 3- Send data to client
        if (no_errors)
        {
            void* file_data = (cached_file != NULL) ? cached_file->data : file_buffer;
            long file_length = (cached_file != NULL) ? cached_file->file_size : file_size;

            // Inform client of file size
            memset(buffer, '\0', BUFFER_SIZE);
            sprintf(buffer, "FILESIZE %ld\n", file_length);
            write(client_sockfd, buffer, strlen(buffer));


            // Send file data
            int c = write(client_sockfd, file_data, file_length);
            if (c < 0)
            {
                perror("write");
                printf("Error sending data to client\n");
            }


            printf("Cache %s. %s sent to the client\n", (cached ? "hit" : "miss"), file_name);

            // Free used memory
            if (cached_file == NULL)
                free(file_buffer);

        }

        // -------------------------------------------------
        // 3 - Close connection
        close(client_sockfd);
    }

    return 0; 
}


// ----------------------
// Utility function to display correct usage and quit
void usage()
{
    printf("Usage: tcp_server listen_port file_directory\n");
    exit(0);
}


