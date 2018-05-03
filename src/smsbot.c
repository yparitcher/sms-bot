/****
Copyright (c) 2018 Y Paritcher
****/

#include "config.h"
#include "smtp.h"
#include "modules.h"
#include "imap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <systemd/sd-daemon.h>

int term = 0;

void sighandle(int signum)
{
    term = 1;
    printf("<5>terminating with signal %d\n", signum);
}

int doit(char *reciever, char *response, Configargs * conf)
{
    Smtp_args p = {
        reciever,
        response,
        conf
    };

    int ret = smtp(&p);
    return ret;
}

int main()
{
    int ret = 0;

 	Configargs *conf = config();
	if (!conf){
		printf("<3>Invalid configuration file\n");
		return 99;
	}
    signal(SIGTERM, sighandle);
    sd_notify (0, "READY=1");


    while(term == 0)
    {
        imap_init();
        int msgnum = 0;
        int counter = 60;
        while(counter--)
        {
            for (msgnum = imap_checkbox(conf); msgnum > 0; msgnum = imap_checkbox(conf)){
                char *from = imap_from(conf, msgnum);
                char *body = imap_body(conf, msgnum);
                imap_store(conf, msgnum);
                if (!strncmp(body, "#weather", 8) && strlen(body) == 14){
                    char *zip = calloc(6,1);
                    strncpy(zip, body+9, 5);
                    char *w = weather(conf, zip);
                    /*printf("%s\n", w);*/
                    if (w != NULL){
                    	ret = doit(from, w, conf);
                    }
                } else if (!strncmp(body, "#forecast", 9)){
                    int cycles = 0;
                    switch (strlen(body)) {
                    case 17:
                        cycles = strtol(body+16, NULL, 10) * 2;
                        if (0 < cycles && cycles < 20){
                            break;
                        }
                    case 15:
                        cycles = 4;
                        break;
                    }
                    if (cycles){
                        char *zips = calloc(6,1);
                        strncpy(zips, body+10, 5);
                        char *f = forecast(conf, zips, cycles);
                        /*printf("%s\n", w);*/
	                    if (f != NULL){
                        	ret = doit(from, f, conf);
                        }
                    }

                }
                free(from);
                free(body);
            }
            if (term == 1)
            {
                break;
            }
            sleep(10);
        }
        imap_cleanup();
        config_free(conf);
    }
    return ret;
}
