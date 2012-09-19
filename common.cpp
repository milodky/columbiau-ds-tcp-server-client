#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

void fatalPError(const char* msg)
{
    perror(msg);
    exit(1);
}

void fatalError(const char* msg)
{
    printf("%s\n", msg);
    exit(1);
}



int readFile(FILE* f, const void* out_data)
{
    char* pointer = (char*)out_data;
    long totalRead = 0;
    int n;
    while (! feof(f) )
    {
        n = fread(pointer, FILE_BLOCK, 1, f);

        pointer += (n * FILE_BLOCK);
        totalRead += (n * FILE_BLOCK);

        if (n == 0 && ! feof(f))
        {
            perror("fread");
            printf("Error while reading file.\n");                    
            return -1;
        }

    }
    return 0;
}



int readLine(int sockfd, void* buffer, size_t length)
{

    // Reset buffer
    memset(buffer, '\0', INET_ADDRSTRLEN);

    size_t totalRead = 0;
    size_t bytesRead = 0;
    char* buf = (char*)buffer;
    while (true)
    {   
        char c;
        bytesRead = read(sockfd, &c, 1);

        if (bytesRead < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (bytesRead == 0)
        {
            if (totalRead == 0)
                return 0;
            break;
        }
        else
        {
            if (c == '\n')
                break;

            if (totalRead < length - 1)
            {
                *buf = c;
                buf++;
                totalRead++;
            }

        }
    }
    *buf = '\0';
    return 0;

}

