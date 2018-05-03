/****
Copyright (c) 2018 Y Paritcher
****/

#ifndef WEATHER_H
#define WEATHER_H

char *weather(Configargs * conf, char *zip);
char *forecast(Configargs * conf, char *zips, int cycles);
char *wiki(char *query);

#endif
