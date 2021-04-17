
CC = g++
CFLAGS  = -g -Wall -Wextra


all: server


server: server.cpp parser.o request.o
	$(CC) $(CFLAGS) -o server server.cpp parser.o request.o

parser.o: parser.cpp parser.h
	$(CC) $(CFLAGS) -c parser.cpp

request.o: request.cpp request.h
	$(CC) $(CFLAGS) -c request.cpp

#count:  countwords.o counter.o scanner.o
#	$(CC) $(CFLAGS) -o count countwords.o counter.o scanner.o
#
## To create the object file countwords.o, we need the source
## files countwords.c, scanner.h, and counter.h:
##
#countwords.o:  countwords.c scanner.h counter.h
#	$(CC) $(CFLAGS) -c countwords.c
#
## To create the object file counter.o, we need the source files
## counter.c and counter.h:
##
#counter.o:  counter.c counter.h
#	$(CC) $(CFLAGS) -c counter.c
#
## To create the object file scanner.o, we need the source files
## scanner.c and scanner.h:
##
#scanner.o:  scanner.c scanner.h
#	$(CC) $(CFLAGS) -c scanner.c

clean:
	$(RM) server *.o
