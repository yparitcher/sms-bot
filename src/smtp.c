/****
Copyright (c) 2018 Y Paritcher
****/

/* functions for sending response */

#include <string.h>
#include <stdlib.h>
#include "config.h"
#include <curl/curl.h>
#include "smtp.h"

char *lineone;
char *linetwo;
char *linethree = "\r\n";
char *linefour;
char *linefive = NULL;
char *payload_text[5];
char *mmsrcpt;
 
struct upload_status {
  int lines_read;
};

/* callback function to convert text buffer to curl data */ 
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}

/* send the response via SMTP */
int smtp(Smtp_args *args)
{

  /* set curl options */
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *mailrcpt;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;

  mailrcpt = NULL;

  curl_global_init(CURL_GLOBAL_ALL);
  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_URL, "smtps://smtp.gmail.com:465/");
  curl_easy_setopt(hnd, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(hnd, CURLOPT_USERPWD, args->conf->usrpwd);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.35.0");
  /*curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L); */
  curl_easy_setopt(hnd, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  curl_easy_setopt(hnd, CURLOPT_MAIL_FROM, args->conf->email);
  curl_easy_setopt(hnd, CURLOPT_READFUNCTION, payload_source);
  curl_easy_setopt(hnd, CURLOPT_READDATA, &upload_ctx);

  /* setup the response data with our info */
  linetwo = malloc(15+args->conf->emaillen);
  snprintf(linetwo, 15+args->conf->emaillen, "From: <%s> Bot\r\n", args->conf->email);
  payload_text[1] = linetwo;
  payload_text[2] = linethree;
  payload_text[4] = linefive;

  /* check if response fits into a text (160 char) or else send via mms */
  if ((int)strlen(args->body) <= 157){
    mailrcpt = curl_slist_append(mailrcpt, args->reciever);
    curl_easy_setopt(hnd, CURLOPT_MAIL_RCPT, mailrcpt);
    lineone = malloc(9+strlen(args->reciever));
    snprintf(lineone, 9+strlen(args->reciever), "To: <%s>\r\n", args->reciever);
    payload_text[0] = lineone;
    linefour = malloc(3+strlen(args->body));
    snprintf(linefour, 3+strlen(args->body), "%s\r\n", args->body);
    payload_text[3] = linefour;

    ret = curl_easy_perform(hnd);

  } else {
    mmsrcpt = calloc(1+args->conf->mmslen+strcspn(args->reciever, "@"), sizeof(char));
    strncpy(mmsrcpt, args->reciever, strcspn(args->reciever, "@"));
    strncat(mmsrcpt, args->conf->mms, 1+args->conf->mmslen);
    mailrcpt = curl_slist_append(mailrcpt, mmsrcpt);
    curl_easy_setopt(hnd, CURLOPT_MAIL_RCPT, mailrcpt);
    lineone = malloc(9+strlen(mmsrcpt));
    snprintf(lineone, 9+strlen(mmsrcpt), "To: <%s>\r\n", mmsrcpt);
    payload_text[0] = lineone;

    /* give option for multiple mms if longer than 2000 chars */
    if ((int)strlen(args->body) <= 2000){
        linefour = malloc(3+strlen(args->body));
        snprintf(linefour, 3+strlen(args->body), "%s\r\n", args->body);
        payload_text[3] = linefour;

        ret = curl_easy_perform(hnd);
    } else {
        linefour = malloc(3+strlen(args->body));
        snprintf(linefour, 3+strlen(args->body), "%s\r\n", args->body);
        payload_text[3] = linefour;

        ret = curl_easy_perform(hnd);
    }
  }

  /* cleanup */
  free(linetwo);
  free(linefour);
  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(mailrcpt);
  mailrcpt = NULL;
  curl_global_cleanup();
  free(lineone);
  free(mmsrcpt);
  free(args->body);

  return (int)ret;
}

