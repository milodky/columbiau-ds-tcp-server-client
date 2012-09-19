#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "cache.h"

bool containsFile(cache* cache, std::string file_path)
{

	// if no entry exists, just returns false
	if (cache->cache_map.count(file_path) == 0)
		return false;

	// retrieve cached entry
	struct cache_entry* cached_file = cache->cache_map[file_path];

	// make sure cache entry is up to date
	struct stat st;
    if (stat(file_path.c_str(), &st) == 0)
    	return st.st_mtime == cached_file->modified_time;

    // file does not exist on file syste, but it exists in cache
    // in this case, return cached file
	return true;
}


const struct cache_entry* cacheFile(cache* cache, std::string file_path, const void* in_contents)
{

	// declarations
	struct cache_entry* entry;
	struct stat st;

	// if file does not exist on file system, return null
    if (stat(file_path.c_str(), &st) != 0)
    	return NULL;

    // initilalize needed memory spaces
    entry = (cache_entry*)malloc(sizeof(struct cache_entry));
    if (entry == NULL)
    {
    	perror("malloc");
    	return NULL;
    }

    // check whether there's enough space in cache, if not, keep removing entries until enough space is avialable
    if (cache->cache_size + st.st_size > CACHE_SIZE)
    {
    	while ( cache->cache_size + st.st_size > CACHE_SIZE )
    	{
    		invalidateLRU(cache);
    		if (cache->cache_count == 0) // cache is empty and still not enough entry, this item will not be cached
    			return NULL;

    	}
    }

    entry->key = (char*)malloc(sizeof(char) * file_path.length());
    if (entry->key == NULL)
    {
    	perror("malloc");
    	free(entry);
    	return NULL;
    }

    strcpy(entry->key, file_path.c_str());
	entry->modified_time = st.st_mtime;
	entry->file_size = st.st_size;
	entry->data = malloc(entry->file_size);
	if (entry->data == NULL)
	{
		free(entry);
		return NULL;
	}

	// copy file contents into cache entry
	memcpy(entry->data, in_contents, entry->file_size);
	cache->cache_map[file_path.c_str()] = entry;

	// update cache metadata
	cache->cache_count++;
	cache->cache_size += entry->file_size;
	// cache->cache_entries.push_front(entry);

	// return pointer to cached entry buffer
	return entry;

}


const struct cache_entry* getCachedFile(cache* cache, std::string file_path)
{
	if ( ! containsFile(cache, file_path))
		return NULL; // file is no in cached

	struct cache_entry* entry = cache->cache_map[file_path.c_str()];
    
    // move entry to beginning of LRU list
    std::list<cache_entry*>::iterator p = find(cache->cache_entries.begin( ), cache->cache_entries.end( ), entry);
    cache->cache_entries.push_front(entry);

	return entry;

}


size_t invalidateLRU(cache* cache)
{
	cache_entry* entry = cache->cache_entries.back();
	cache->cache_count--;
	cache->cache_size -= entry->file_size;
	cache->cache_map.erase(std::string(entry->key));
	cache->cache_entries.pop_back();
	free(entry);
	return cache->cache_size;
}

