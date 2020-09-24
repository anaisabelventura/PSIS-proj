/******************************************************************************
 * File Name: bot.c
 *        (c) 2019 PSIS
 * Authors:    Ana Isabel Ventura, Tiago Marques
 * Last modified: 31/05/2019
 *
 * COMMENTS
 *    
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <malloc.h>

#include <netinet/in.h>

#include <pthread.h>

#include "board_library.h"

#include <fcntl.h>

int main( int argc, char * argv[]){
	/********************************* DEFINING VARIABLES *************************/
	int done = 0;
	int connection_status = 0;
	struct server_message_struct arg;
	char server_response[128];

	/********************************* STARTS CLIENT SERVER CONNECTION *************************/
	//create a socket
	int network_socket;
	network_socket = socket(AF_INET, SOCK_STREAM, 0);
	arg.network_socket = network_socket;
	// specify an address for the socket
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(3000); /* htons converts int to correct port format */
	server_address.sin_addr.s_addr = INADDR_ANY;
	//connect socket
	connection_status = connect(network_socket, (struct  sockaddr *) &server_address, sizeof(server_address));
	// Checks if there are connection errors
	if(connection_status == -1){
		printf("Connection Error with the remote socket \n Trying again\n");
		return EXIT_FAILURE;
	}

	arg.bot=init_bot();

	/* Receives data from the server */
	while(recv(network_socket, &server_response, 128*sizeof(char), 0) < 0)
		continue;
	printf("SERVER: %s\n", server_response);

	/* Sets socket to non blocking mode */
	fcntl(network_socket, F_SETFL, O_NONBLOCK);

	/*Iniciate Thread that is always for messanges from server */
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, server_messages, &arg);


	arg.code = -1;
	arg.server_commands[1] = -1;
	arg.server_commands[2] = -1;
	arg.server_commands[3] = -1;
	arg.server_commands[4] = -1;


	while (!done){
		//done = check_win(network_socket, bot);
	}

	printf("Exiting\n");
	//free alocated memory
	//bot_free(bot);
	//Closes socket
	close(network_socket);

	return EXIT_SUCCESS;
}
