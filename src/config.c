/****
Copyright (c) 2018 Y Paritcher
****/

/* functions to load the config file via ini library */

#include "config.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

Configargs conf;

/* check that the weather ungerground key is in valid format */
int check_key(const char *key){
	if (strlen(key) == 16){
		for (int i = 0; i < 16; i++){
			if (!isxdigit(*(key+i))){
				return 0;
			}
		}
		return 1;
	}
	return 0;
}

/* load configuration */
Configargs * config()
{
	conf.config = ini_load("/etc/smsbot/smsbot.conf");
	if (!conf.config){
	return NULL;
	}
	conf.email = ini_get(conf.config, "user", "email");
	conf.usrpwd = ini_get(conf.config, "user", "userpasswd");
	conf.mms = ini_get(conf.config, "user", "mmsdomain");
	conf.wukey = ini_get(conf.config, "services", "wukey");
	/* check if all options are set */
	if (conf.config && conf.email && conf.usrpwd && conf.mms && check_key(conf.wukey)){
		conf.emaillen = strlen(conf.email);
		conf.mmslen = strlen(conf.mms);
		return &conf;
	} else{
		config_free(&conf);
		return NULL;
	}
}

/* cleanup */
void config_free(Configargs * freeconf)
{
	ini_free(freeconf->config);
}
