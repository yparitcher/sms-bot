/****
Copyright (c) 2018 Y Paritcher
****/

/* functions to get and parse a response based on the qwery */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include <curl/curl.h>
#include "parson.h"
#include "modules.h"

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

/* weather function */
char *weather(Configargs * conf, char *zip)
{
    /* get the data */
    char *result = NULL;
    char url[78];
    char url_base[] = "http://api.wunderground.com/api/%s/conditions/pws:0/q/%s.json";

    snprintf(url, 79, url_base, conf->wukey, zip);
    free(zip);

    char *buffer = curldo(url);

    /* parse it */
    JSON_Value *root_value;
    JSON_Object *root_object;

    root_value = json_parse_string(buffer);
    if (json_value_get_type(root_value) != JSONObject) {
        return result;
    }

    root_object = json_value_get_object(root_value);

    size_t len = snprintf( NULL, 0, "%s is %s today, %.0f F %s humidity, wind is %s\n",
         json_object_dotget_string(root_object, "current_observation.display_location.full"),
         json_object_dotget_string(root_object, "current_observation.weather"),
         json_object_dotget_number(root_object, "current_observation.temp_f"),
         json_object_dotget_string(root_object, "current_observation.relative_humidity"),
         json_object_dotget_string(root_object, "current_observation.wind_string"));
    result = malloc(len+1);
    snprintf( result, len+1, "%s is %s today, %.0f F %s humidity, wind is %s\n",
         json_object_dotget_string(root_object, "current_observation.display_location.full"),
         json_object_dotget_string(root_object, "current_observation.weather"),
         json_object_dotget_number(root_object, "current_observation.temp_f"),
         json_object_dotget_string(root_object, "current_observation.relative_humidity"),
         json_object_dotget_string(root_object, "current_observation.wind_string"));
    json_value_free(root_value);
    free(buffer);

    return result;
}

/* forecast function */
char *forecast(Configargs * conf, char *zip, int cycles)
{
    /* get the data */
    char *result = NULL;
    char url[81];
    char *url_base = "http://api.wunderground.com/api/%s/forecast10day/pws:0/q/%s.json";
    snprintf(url, 82, url_base, conf->wukey, zip);
    free(zip);

    char *buffer = curldo(url);

    /* parse it */
    JSON_Value *root_value;
    JSON_Object *root_object;
    JSON_Array *fctdays;
    JSON_Object *day;

    root_value = json_parse_string(buffer);
    if (json_value_get_type(root_value) != JSONObject) {
        return result;
    }

    size_t size = 0;
    root_object = json_value_get_object(root_value);
    fctdays = json_object_dotget_array(root_object, "forecast.txt_forecast.forecastday");
    for (int i =0; i < cycles; i++){
        day = json_array_get_object(fctdays, i);
        size_t len = snprintf( NULL, 0, "%s: %s \n",
            json_object_get_string(day, "title"),
            json_object_get_string(day, "fcttext"));
        result = realloc(result, size + len);
        snprintf( result + size, len, "%s: %s \n",
            json_object_get_string(day, "title"),
            json_object_get_string(day, "fcttext"));
        size += len-1;
    }
    json_value_free(root_value);
    free(buffer);

    return result;    
}

/* wikipedia function (coming soon)*/
/*char *wiki(char *query)
{
    char *url = "https://en.wikipedia.org/api/rest_v1/page/summary/";


}*/
