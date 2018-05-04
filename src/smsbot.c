/****
Copyright (c) 2018 Y Paritcher
****/

/* daemon function to run the thing */

#include "config.h"
#include "smtp.h"
#include "weather.h"
#include "imap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <systemd/sd-daemon.h>

int term = 0;

/* callback to gracefully close on recieving stop from systemd */
void sighandle(int signum)
{
    term = 1;
    printf("<5>terminating with signal %d\n", signum);
}

/* send the response */
int sendit(char *reciever, char *response, Configargs * conf)
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

 	/* load config */
 	Configargs *conf = config();
	if (!conf){
		printf("<3>Invalid configuration file\n");
		return 99;
	}
    /* register callback for TERM signal */
    signal(SIGTERM, sighandle);
    /* let systemd know we started up okay */
    sd_notify (0, "READY=1");


    /* daemon loop */
    while(term == 0)
    {
        imap_init();
        int msgnum = 0;
        int counter = 60;
        /* reset imap session ever 'counter' times (balance between less calls and stale sessions) */
        while(counter--)
        {
            /* check for new query and act on it */
            for (msgnum = imap_checkbox(conf); msgnum > 0; msgnum = imap_checkbox(conf)){
                char *from = imap_from(conf, msgnum);
                char *body = imap_body(conf, msgnum);
                imap_store(conf, msgnum);
                /* if match weather */
                if (!strncmp(body, "#weather", 8) && strlen(body) == 14){
                    char *zip = calloc(6,1);
                    strncpy(zip, body+9, 5);
                    char *w = weather(conf, zip);
                    if (w != NULL){
                    	ret = sendit(from, w, conf);
                    }
                /* if match forecast */
                } else if (!strncmp(body, "#forecast", 9)){
                    /* check for amount of days */
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
                        if (f != NULL){
                        	ret = sendit(from, f, conf);
                        }
                    }

                }
                free(from);
                free(body);
            }
            /* end if recieved signal */
            if (term == 1)
            {
                break;
            }
            /* how long to wait between checking for new query */
            /* make sure to set the wathdog option accordingly in smsbot.service*/
            sleep(10);
        }
        /* cleanup */
        imap_cleanup();
        config_free(conf);
    }
    ret = 0;
    return ret;
}
