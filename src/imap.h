#ifndef IMAP_H
#define IMAP_H

void imap_init();
void imap_cleanup();
int imap_checkbox(Configargs * conf);
char *imap_from(Configargs * conf, int msgnum);
char *imap_body(Configargs * conf, int msgnum);
void imap_store(Configargs * conf, int msgnum);

#endif
