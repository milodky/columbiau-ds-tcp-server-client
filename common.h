#ifndef w4995_lab0_common_h
#define w4995_lab0_common_h

#define FILE_BLOCK 	1024  // how many bytes to attempt to read via a single read syscall

int readLine(int sockfd, void * buffer, size_t length); // reads a line off of a stream socket (excluding the terminating \n)
void fatalPError(const char* msg); // prints out errno friendly message and exits
void fatalError(const char* msg); // prints out an error message and exits
int readFile(FILE* f, const void* out_data); // reads a file's contents into  out_data

#endif
