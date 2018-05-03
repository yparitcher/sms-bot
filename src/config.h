/****
Copyright (c) 2018 Y Paritcher
****/

#ifndef CONFIG_H
#define CONFIG_H

#include "ini.h"

typedef struct configargs_t{ 
  ini_t *config;
  const char *email;
  int emaillen;
  const char *usrpwd;
  const char *mms;
  int mmslen;
  const char *wukey;
} Configargs;

Configargs * config();

void config_free(Configargs * conf);

#endif
