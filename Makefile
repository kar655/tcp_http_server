
CC = g++
CFLAGS  = -Wall -Wextra -std=c++17 -O2

all: serwer

serwer: serwer.cpp parser.o requests.o correlatedServer.o httpExceptions.h
	$(CC) $(CFLAGS) -o serwer serwer.cpp parser.o requests.o correlatedServer.o

parser.o: parser.cpp parser.h
	$(CC) $(CFLAGS) -c parser.cpp

requests.o: requests.cpp requests.h
	$(CC) $(CFLAGS) -c requests.cpp

correlatedServer.o: correlatedServer.cpp correlatedServer.h
	$(CC) $(CFLAGS) -c correlatedServer.cpp

clean:
	$(RM) serwer *.o
