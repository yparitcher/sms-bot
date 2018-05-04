/****
Copyright (c) 2018 Y Paritcher
****/

/* functions to check email for new query */

#include "config.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-daemon.h>
#include "imap.h"

struct MemoryStruct {
  char *memory;
  size_t size;
};
struct MemoryStruct chunk;
CURLcode ret;
CURL *hnd;
 
/* callbacj to convert curl data to text buffer */
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

/* initalize imap session */
void imap_init()
{
    hnd = curl_easy_init();
}

/*cleanup */
void imap_cleanup()
{
    curl_easy_cleanup(hnd);
    hnd = NULL;
    free(chunk.memory);
    chunk.memory = NULL;
    curl_global_cleanup();
}

/* check the email box for unread msg */
int imap_checkbox(Configargs * conf)
{
    /* let systemd know we are still alive not failed (to allow restart on hang) */
    sd_notify(0,"WATCHDOG=1");
    
    /* do the curl stuff */
    chunk.memory = realloc(chunk.memory, 1);
    chunk.size = 0;

    curl_easy_reset(hnd);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_USERPWD, conf->usrpwd);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
	/*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(hnd, CURLOPT_URL, "imaps://imap.gmail.com/INBOX");
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "SEARCH RETURN (MIN) UNSEEN");
    ret = curl_easy_perform(hnd);

	/* return lowest unread message */
    int msg = 0;
    if(27 < chunk.size)
    {
        msg = strtol(chunk.memory+27, NULL, 10);
    }
    return msg;
}

/* get the senders email address to respond to*/
char *imap_from(Configargs * conf, int msgnum)
{
	int urllen = snprintf(NULL, 0, "%d", msgnum)+66;
    char *urlbase = "imaps://imap.gmail.com/INBOX/;UID=%d;SECTION=HEADER.FIELDS%20(from)";
    char *url = malloc(urllen);
    snprintf(url, urllen, urlbase, msgnum);

    chunk.memory = realloc(chunk.memory, 1);
    chunk.size = 0;

    curl_easy_reset(hnd);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_USERPWD, conf->usrpwd);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
	/*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    free(url);
    ret = curl_easy_perform(hnd);

    char *from = calloc(chunk.size-9,1 );
    strncpy(from, chunk.memory+6, chunk.size-10);
    return from;
}

/* get email body AKA our qwery */
char *imap_body(Configargs * conf, int msgnum)
{
	int urllen = snprintf(NULL, 0, "%d", msgnum)+48;
    char *urlbase = "imaps://imap.gmail.com/INBOX/;UID=%d;SECTION=TEXT";
    char *url = malloc(urllen);
    snprintf(url, urllen, urlbase, msgnum);

    chunk.memory = realloc(chunk.memory, 1);
    chunk.size = 0;

    curl_easy_reset(hnd);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_USERPWD, conf->usrpwd);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);
    /*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    free(url);
    ret = curl_easy_perform(hnd);

    char *body = calloc(chunk.size-1,1 );
    strncpy(body, chunk.memory, chunk.size-2);
    return body;
}

/* mark message as read so as not to use twice */
void imap_store(Configargs * conf, int msgnum)
{
	int urllen = snprintf(NULL, 0, "%d", msgnum)+20;
    char *reqbase = "STORE %d +FLAGS \\Seen";
    char *req = malloc(urllen);
    snprintf(req, urllen, reqbase, msgnum);

    curl_easy_reset(hnd);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.47.0");
    curl_easy_setopt(hnd, CURLOPT_USERPWD, conf->usrpwd);
	/*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
    curl_easy_setopt(hnd, CURLOPT_URL, "imaps://imap.gmail.com/INBOX");
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, req);
    free(req);
    ret = curl_easy_perform(hnd);
}

