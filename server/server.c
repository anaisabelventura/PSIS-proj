/******************************************************************************
 * File Name: server.c
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
	/*  ------------------ DEFINING VARIABLES -----------------*/
	SDL_Event event;
	int done = 0;
	struct sockets_list *sockets;			/* [0]=server_Address  [i>0]=client_sockets */
	sockets = (sockets_list *)malloc(sizeof(sockets_list));
	struct server_wait_conn_struct arg;
	arg.head = sockets;
	int port_number;
	int board_dim;

	/* Syntax Checks */
	if(argv[1]== NULL || argc<2){
		printf("\n\n\nThe correct sintax for starting the server is: ./runserver BOARD_DIMENSION\nExample: ./runserver 4\n\n\n");
		return EXIT_FAILURE;
	}
	port_number = 3000;
	board_dim = atoi(argv[1]);

	if(board_dim < 2 || board_dim%2 != 0 ){
		printf("Board dimensions must be at least 2x2 and an even number, please start the server again.\n");
		return EXIT_FAILURE;
	}
	
	/*  ------------------ SERVER ON -----------------*/
	// Creates server socket
	int server_socket;
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Error socket\n");
		return EXIT_FAILURE;
	}

	/* Defines server address */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port_number);
	server_address.sin_addr.s_addr = INADDR_ANY;
	//Bind socket to our specified IP and port
	if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1){
		printf("Error bind\n");
		return EXIT_FAILURE;
	}
	/* Listen for connections */
	if(listen(server_socket, 5) == -1){
		printf("Error listen\n");
		return EXIT_FAILURE;
	}
	sockets->socket_id = server_socket;
	sockets->player_id = 0;

	/* Sets socket to non blocking mode */
	fcntl(server_socket, F_SETFL, O_NONBLOCK);
	

	/*  ------------------ GRAPHICAL UI ON -----------------*/
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		 printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		 exit(-1);
	}

	if(TTF_Init()==-1) {
			printf("TTF_Init: %s\n", TTF_GetError());
			exit(2);
	}

	/************************************ STARTS GAME *************************************/
	create_board_window(300, 300,  board_dim);
	init_board(board_dim);

	/* Iniciate Thread that is always waiting for new players */
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, server_wait_conn, &arg);
	
	while (!done){
		while (SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				done = 1;
				break;
			}
		}
	}
	printf("END\n");
	close_board_windows();
	close_clients();
	return EXIT_SUCCESS;
}