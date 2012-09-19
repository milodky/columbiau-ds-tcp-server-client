----------------------------------------------
Fundamentals of Distributed Systems -  Lab 0	
----------------------------------------------
Done by:   Aiman Najjar
UNI:            an2434
----------------------------------------------


            Table of Content

 0  Table of Content
 1  Build Instructions
 2  Usage Examples
 3  Source Code & Implementation Description
 4  Test Cases


----------------------------------------------
1   Build Instructions
----------------------------------------------
To build, simply run `make all`.
To install server only, run `make server`, similarly to install client only, run `make client` 
To remove compiled binaries, simply run `make clean`

----------------------------------------------
2   Usage Examples
----------------------------------------------
The server side takes 2 parameters - the port to listen on and the directory to find the file. It looks like this:

% tcp_server port_to_listen_on file_directory
e.g.
% ./tcp_server 9089 /home/dist/lab0


The client side takes 4 parameters - the server name, server port, the file to request, and the local directory to save the file in. It looks like this:

% tcp_client server_host server_port file_name directory 
e.g.
% ./tcp_client 59.78.58.28 9089 lab0.html . 


----------------------------------------------
3   Source Code & Implementation Description
----------------------------------------------
The server side contains the following source files (and their header files):
	tcp_server.cpp - contains the entry point the core server implmentation (including the protocol and socket implementation)
	cache.cpp - contains the cache implemntation

The client side contains the following source file:
	tcp_client.cpp - contains the entry point the core server implmentation (including the protocol and socket implementation)

Both server and client make use of common utility functions defined at common.cpp, which basically contains convenient I/O wrappers (to read/write from/to sockets and files)


	## 3.1 Cache Implementation (cache.cpp and cache.h) ##

	The cache is implemented in cache.cpp, the stubs and structures are defined at cache.h.
	There is two main structures needed to use the cache: 
		o struct cache
		o struct cache_entry

	There cache size can be tweaked in cache.h, it's declared as a constant named CACHE_SIZE and the value is measured in bytes


	3.1.1 struct cache

	The first structure contains the metadata for the cache which are:
		o cache_count: stores the number of cache entries present in the cache at the moment
		o cache_entries: a list of cache_entry pointers (more on cache_entry below), ordered by most recently used entry (the least used one being the tail)
		o cache_size: the much runtime memory the cache is consuming at the moment
		o cache_map: contains key-value pairs where key is the full path to file being cached and value is a pointer to a cache entry



	3.1.2 struct cache_entry

	Defines metadata for a single cache entry, also contains a pointer to the data buffer:
		o key: The key used to access this cache entry in the cache map (i.e. the full path to the cached file)
		o modified_time: the last modified time stamp of the file at the time it was cached (used to check whether file was changed since it's been cached)
		o file_size: the size of the cached file
		o data: pointer to the file contents buffer in the heap


	3.1.3 Cache functions

	The following functions are defined in cache.cpp to make use of cache struct:
		- bool containsFile(std::string file_path);
		Given a pointer to cache struct, returns true if specified file name is cached, false otherwise
		A file is determined to be in cached if the cache contains an up-to-date version of it in the cache

		- const struct cache_entry* cacheFile(cache* cache, std::string file_path, const void* in_contents);
		Inserts a cache entry into cache, returns a pointer to cache entry on success
		returns NULL on failure


		- const struct cache_entry* getCachedFile(cache* cache, std::string file_path);
		Retrieves a cached file, returns NULL if cache entry does not exist
		returns a pointer to cache entry


		size_t invalidateLRU(cache* cache);
		Removes the least recently used entry in the cache
		and returns the new cache size


	## 3.2 Server & Client Implementation ##

	Server and client sides are implemented in tcp_server.cpp and tcp_client.cpp, respectively.
	The implementation is straightforward and it makes use of UNIX I/O socket system calls.
	The readLine() function is commonly used between the server and client, it reads off of the specified socket descriptor
	until a '\n' characater is encoutered. The client-server protocol is as follows.


		o Server listens on incoming port
		o Client initiates connection and sends "REQ xxxx" where xxxx is the name of the requested file
		o Server responds with one of the following
			* ERROR: If file does not exist or there was an I/O error when it attempted to read it
			* ERRORSIZE: If file size is larger than maximum allowed file size (which is, MAX_FILE_SIZE in tcp_server.cpp)
			* FILESIZE XXXX: If file is ready to transmit file, XXXX represents the file size
		o Client, based on the server response, either attempts to the read the incoming file, or prints out an error message
		o Client exists
		o Server accepts the next incoming connection

----------------------------------------------
3   Test Cases
----------------------------------------------
I have ran the following test cases:

	o Attempting to connect to a dead server
	o Attempting to request a file that is larger than MAX_FILE_SIZE (defined in tcp_server.cpp)
	o Attempting to request a file that does not exist in server's specified lookup directory
	o Attempting to listen on invalid port
	o Attempting to send malformed protocol messages (by directly connecting using telnet)
	o Attempting to retrieve a file and save it in a directory where there isn't sufficient permissions to save
	o Attempting to request a file that exists in server


