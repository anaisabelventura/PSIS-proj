# 
#  File Name: Makefile
# 	      (c) 2019 PSIS
#  Authors:    Ana Isabel Ventura, Tiago Marques
#  Last modified: 21/05/2018
# 
#  COMMENTS
# 		Description: Makefile for 
# 		Usage : 
# 				1) to genenrte the executable file for the bot program, run
# 			 		$ make runbot
# 				2) to clean all object files and the executable file, run 
# 					$ make clean
# Compiler
CC=gcc
# Compiler Flags 
CFLAGS= -I/usr/local/include -Wall -g -pedantic -std=c99 

LFLAGS= -pthread 

runbot: bot.o board_library.o 
	$(CC) $(CFLAGS) -o runbot bot.o board_library.o $(LFLAGS)

bot: bot.c 
		$(CC) -c bot.c $(CFLAGS) $(LFLAGS)

board_library.o: board_library.c board_library.h
	$(CC) -c  board_library.c  $(CFLAGS) $(LFLAGS)

clean:
	rm -f *.o core a.out runbot *~