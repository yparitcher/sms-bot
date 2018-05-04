CC=gcc
CFLAGS=-Wall -Wextra -pedantic $(INC_DIR:%=-I%)
LDLIBS=-lcurl -lsystemd

INC_DIR = parson ini/src

VPATH = src $(INC_DIR)

objects = smtp.o weather.o query.o parson.o imap.o config.o ini.o

.PHONY: clean

smsbot: smsbot.o $(objects)

clean:
	rm -f smsbot smsbot.o $(objects)
