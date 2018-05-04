/****
Copyright (c) 2018 Y Paritcher
****/

/* wrapper functions to get and parse a response based on the qwery */
/* weather and forecast */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "parson.h"
#include "query.h"
#include "weather.h"

/* current weather function */
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
