# 
#  File Name: Makefile
# 	      (c) 2019 PSIS
#  Authors:    Ana Isabel Ventura, Tiago Marques
#  Last modified: 21/05/2019
# 
#  COMMENTS
# 		Description: Makefile for 
# 		Usage : 
# 				1) to genenrte the executable file for the server program, run
# 			 		$ make runserver
# 				2) to clean all object files and the executable file, run 
# 					$ make clean
# Compiler
CC=gcc
# Compiler Flags 
CFLAGS= -I/usr/local/include -Wall -pedantic -std=c99
LFLAGS= -pthread -lSDL2 -lSDL2_ttf 

runserver: server.o board_library.o UI_library.o
	$(CC) $(CFLAGS) -o runserver server.o board_library.o UI_library.o $(LFLAGS)

server: server.c board_library.h UI_library.h
	$(CC) -c  server.c $(CFLAGS) $(LFLAGS)

board_library.o: board_library.c board_library.h
	$(CC) -c  board_library.c  $(CFLAGS) $(LFLAGS)

UI_library.o: UI_library.c UI_library.h
	$(CC) -c UI_library.c $(CFLAGS) $(LFLAGS)

clean::
	rm -f *.o core a.out runserver *~
