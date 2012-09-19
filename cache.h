#ifndef w4995_lab0_cache_h
#define w4995_lab0_cache_h

#include <time.h>
#include <string>
#include <sys/stat.h>
#include <map>
#include <list>

// Desired cache size in bytes
#define CACHE_SIZE	60 * 1024 * 1024 // 60mb

// Cache entry struct
struct cache_entry
{
	char* key; // cache key
	time_t modified_time; // last modified timestamp
	size_t file_size; // size of cached file contents 
	void* data; // cached file contents
};

// Cache struct
struct cache
{
	int cache_count; // count of cache entries
	std::list<cache_entry*> cache_entries; // list of cache entries ordered by most recently used (least used entry is the tail)	
	size_t cache_size;
	std::map<std::string, struct cache_entry*> cache_map;
};



// Returns true if file is cached, false otherwise
// cache keys are filenames
// cache values are struct cache_entry instance, which contains
// a last-modified stamp as well as the file contents
// will have separate entries in the cache
bool containsFile(cache* cache, std::string file_path);

// Inserts a cache entry into cache, returns a pointer to cache entry on success
// returns NULL on failure
const struct cache_entry* cacheFile(cache* cache, std::string file_path, const void* in_contents);


// Retrieves a cached file, returns NULL if cache entry does not exist
// returns a pointer to cache entry
const struct cache_entry* getCachedFile(cache* cache, std::string file_path);


// Removes the least recently used entry in the cache
// and returns the new cache size
size_t invalidateLRU(cache* cache);

#endif
