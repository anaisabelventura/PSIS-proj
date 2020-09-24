/******************************************************************************
 * File Name: client.c
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
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <pthread.h>

#include "board_library.h"
#include "UI_library.h"

#include <fcntl.h>

int main( int argc, char * argv[]){
	/********************************* DEFINING VARIABLES *************************/
	SDL_Event event;
	int done = 0;
	int connection_status = 0;
	int co[2];
	struct server_message_struct arg;
	int board_x, board_y;
	char message[128];
	int dim_board;
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

	/* Receives data from the server */
	char server_response[128];
	while(recv(network_socket, &server_response, 128*sizeof(char), 0) < 0)
		continue;
	printf("SERVER: %s\n", server_response);

	/* Receives board from server and initiates it */
	dim_board = init_board(network_socket, &done);
	
	/* Sets socket to non blocking mode */
	fcntl(network_socket, F_SETFL, O_NONBLOCK);


	/*Iniciate Thread that is always for messanges from server */
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, server_messages, &arg);

	/********************************* GRAPHICAL UI ON *************************/
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		exit(-1);
	}
	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	create_board_window(300, 300,  dim_board);/*creates board window  and initializes it for each client*/
	arg.code = -1;
	arg.server_commands[1] = -1;
	arg.server_commands[2] = -1;
	arg.server_commands[3] = -1;
	arg.server_commands[4] = -1;

	while (!done){
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					done = SDL_TRUE;
					strcpy(message, "die");
					while(send(network_socket, message, 128*sizeof(char), 0) < 0){
						continue;
					}
					break;
				}
				case SDL_MOUSEBUTTONDOWN:{ /*Change mouse coordinates to board coordinates*/
					if(arg.click_block == 1)
						continue;
					board_x = 0;
					board_y = 0;
					get_board_card(event.button.x, event.button.y, &board_x, &board_y);
					co[0] = board_x;
					co[1] = board_y;
					printf("click (%d %d) -> (%d %d)\n", event.button.x, event.button.y, board_x, board_y);
					/* Send message to the server with picked coordinates */
					strcpy(message, "coordinates");
					while(send(network_socket, message, 128*sizeof(char), 0) < 0)
						continue;
					while(send(network_socket, co, 2*sizeof(int), 0) < 0)
						continue;
				}
			}
		}
	

	}
	printf("Exiting\n");
	//Closes windows
	close_board_windows();
	//Closes socket
	close(network_socket);

	return EXIT_SUCCESS;
}
