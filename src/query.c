/****
Copyright (c) 2018 Y Paritcher
****/

/* functions to get a response */

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include"query.h"

struct MemoryStruct {
  char *memory;
  size_t size;
};
 
/* callback to convert curl data to text buffer */
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
    /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
 
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
 
    return realsize;
}

/* fetch the info */
char *curldo(char *url)
{
    CURLcode ret;
    CURL *hnd;

    struct MemoryStruct chunk;

    chunk.memory = NULL;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
	/*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

    ret = curl_easy_perform(hnd);
  
    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_global_cleanup();

    if(ret != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(ret));
    }
    return chunk.memory;
}

