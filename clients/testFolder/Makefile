# Makefile

CC = gcc
CFLAGS = -Wall

all: normalClient lossSimulation timeoutTest

normalClient: normalClient.c
	$(CC) $(CFLAGS) -o normalClient normalClient.c

lossSimulation: lossSimulation.c
	$(CC) $(CFLAGS) -o lossSimulation lossSimulation.c

timeoutTest: timeoutTest.c
	$(CC) $(CFLAGS) -o timeoutTest timeoutTest.c

clean:
	rm -f normalClient lossSimulation timeoutTest *.o
