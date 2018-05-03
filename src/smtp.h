#ifndef SMTP_H
#define SMTP_H

typedef struct smtp_args_t{ 
  char *reciever;
  char *body;
  Configargs * conf;
} Smtp_args;

int smtp(Smtp_args *args);

#endif
