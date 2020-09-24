/******************************************************************************
 * File Name: board_library.c
 *        (c) 2019 PSIS
 * Authors:    Ana Isabel Ventura, Tiago Marques
 * Last modified: 31/05/2019
 *
 * COMMENTS
 *    
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <malloc.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <pthread.h>

#include <fcntl.h>

#define _OPEN_SYS

#include "board_library.h"
#include "UI_library.h"

#include <time.h>

int dim_board;
board_place * board;
int n_corrects;
sockets_list *head = NULL;

pthread_mutex_t lock;

int linear_conv(int i, int j){
  return j*dim_board+i;
}
char * get_board_place_str(int i, int j){
  return board[linear_conv(i, j)].v;
}
void init_board(int dim){
  int count  = 0;
  int i, j;
  char * str_place;
  time_t t;
  
  dim_board= dim;
  n_corrects = 0;

  board = malloc(sizeof(board_place)* dim *dim);
  srand((unsigned) time(&t));
	if (pthread_mutex_init(&lock, NULL) != 0){
	printf("\n mutex init failed\n");
	return;
	}

  for( i=0; i < (dim_board*dim_board); i++){
    board[i].v[0] = '\0';
    board[i].player_id = -1;
  }

  for (char c1 = 'a' ; c1 < ('a'+dim_board); c1++){
    for (char c2 = 'a' ; c2 < ('a'+dim_board); c2++){
      do{
        i = rand()% dim_board;
        j = rand()% dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);

      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      do{
        i = rand()% dim_board;
        j = rand()% dim_board;
        str_place = get_board_place_str(i, j);
        printf("%d %d -%s-\n", i, j, str_place);
      }while(str_place[0] != '\0');
      str_place[0] = c1;
      str_place[1] = c2;
      str_place[2] = '\0';
      count += 2;
      if (count == dim_board*dim_board)
        return;
    }
  }
}
play_response board_play(sockets_list *client, int x, int y, int play1[]){
  play_response resp;
  resp.code = 10;
  if(board[linear_conv(x, y)].player_id != -1){
    printf("FILLED\n");
    resp.code =0;
  }else{
    if(play1[0]== -1){
        printf("FIRST\n");
        resp.code =1;

        play1[0]=x;
        play1[1]=y;
        resp.play1[0]= play1[0];
        resp.play1[1]= play1[1];
        strcpy(resp.str_play1, get_board_place_str(x, y));
      }else{
        char * first_str = get_board_place_str(play1[0], play1[1]);
        char * secnd_str = get_board_place_str(x, y);

        if ((play1[0]==x) && (play1[1]==y)){
          resp.code =0;
          printf("FILLED1\n");
        } else{
          resp.play1[0]= play1[0];
          resp.play1[1]= play1[1];
          strcpy(resp.str_play1, first_str);
          resp.play2[0]= x;
          resp.play2[1]= y;
          strcpy(resp.str_play2, secnd_str);

          if (strcmp(first_str, secnd_str) == 0){
            printf("CORRECT!!!\n");
			/*increment number of pairs made by player*/
			client->pairs+=1;
			
            strcpy(first_str, "");
            strcpy(secnd_str, "");

            n_corrects +=2;
            if (n_corrects == dim_board* dim_board)
                resp.code =3;
            else
              resp.code =2;
          }else{
            printf("INCORRECT");

            resp.code = -2;
          }
          play1[0]= -1;
        }
      }
    }
  return resp;
}
void* server_wait_conn(void* arg){
	char server_message[128];
	int socket_id;
	int i=1;
	sockets_list *aux = NULL;
	sockets_list *aux1 = NULL;
	time_t t;

	/* Initiates values for client threads */
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/* Intializes random number generator */
	srand((unsigned) time(&t));

	struct server_wait_conn_struct *arg_struct = (struct server_wait_conn_struct*) arg;
	head = arg_struct->head;
	head->next = NULL; /* Setting next of head to null */
	aux = head;

	/* Sets up this struct so we can send "auxs" to the client threads */
	struct server_wait_conn_struct thread_arg;

	/* Inserts client socket id in the sockets list */
	while(1){
		aux = head;
		sleep(2);
		while((socket_id = accept(head->socket_id, NULL, NULL)) == -1)
			continue;
		printf("Client with id = %d has joined the game!\n", socket_id);
		/* Inserts new node in the end of the list */
		while(aux->next != NULL){
			aux = aux->next;
		}
		aux->next = malloc(sizeof(sockets_list));
		aux = aux->next;
		aux->next = NULL;
		/* Inserts player data */
		aux->socket_id = socket_id;

		aux->player_id = i;
		aux->got_board = 0;
		aux->rgb[0] = rand()%255;
		aux->rgb[1] = rand()%255;
		aux->rgb[2] = rand()%255;
		aux->play1[0] = -1;
		aux->play1[1] = -1;
		aux->pairs=0;
		/* Send message to the client */
		printf("Sends client message\n");
		snprintf(server_message, 128*sizeof(char), "You have connected to the server succesfully\n You are player number %d\n", socket_id);
		while(send(socket_id, server_message, 128*sizeof(char), 0) == -1)
			continue;
		i++;
		if(i==3){
			for( aux1=head->next; aux1!=NULL; aux1=aux1->next){
				/* Sends board to client */
				send(aux1->socket_id, &dim_board, sizeof(int), 0);
			}
		}
		sleep(1);

		if(i>3)
			/* Sends board to client */
			while(send(socket_id, &dim_board, sizeof(int), 0) == -1)
				continue;		

		thread_arg.head = aux;
		pthread_create(&(aux->tid), &attr, client_thread, &thread_arg);

		sleep(1);
		if(i>=3)
			/* Sends information on screen for client */
			get_initial_screen(socket_id);
		
	}
	pthread_exit(0);
}
void get_initial_screen(int socket_id){
	for(int x=0; x<dim_board; x++){
		for(int y=0; y<dim_board; y++){
			if(board[linear_conv(x, y)].player_id != -1){
				//sleep(2);
			    update_clients(socket_id, 4, x, y, x, y, board[linear_conv(x, y)].v, board[linear_conv(x, y)].v); /* Code 4 means this client in socket id is getting board state for the first time */
			}
		}
	}	
}
void* client_thread(void* arg){
	sockets_list *client = NULL;
	sockets_list *aux=NULL;
	struct server_wait_conn_struct *arg_struct = (struct server_wait_conn_struct*) arg;
	client = arg_struct->head;
	aux = arg_struct->head;
	client->first_play_time = 0;
	char server_message[128];
	int co[2];

	printf("Thread for player %d initiated\n", client->socket_id);	

	/* Sets socket to non blocking mode */
	fcntl(client->socket_id, F_SETFL, O_NONBLOCK);

	while(1){
		if(recv(client->socket_id, &server_message, 128*sizeof(char), 0) == -1){
			/* Check if there is 5 seconds past 1st play */
			if(client->first_play_time != 0){
				if((int)time(NULL) - client->first_play_time >= 5){
					printf("5 seconds gone past\n");
					insert_play(co[0], co[1], client->socket_id, client->resp, 1);
					client->first_play_time = 0;
					board[linear_conv( co[0], co[1] )].player_id = -1;
				}
			}
			continue;
		}

		printf("CLIENT:%s\n", server_message);
		if(strcmp(server_message, "coordinates") == 0){	
			while(recv(client->socket_id, co, 2*sizeof(int), 0)  == -1)
				continue;
			printf("coordinates:%d,%d\n",co[0], co[1]);
			insert_play(co[0], co[1], client->socket_id, client->resp, 0);
		}
		else if(strcmp(server_message, "die") == 0){
			delete_player(client->socket_id);
			
			if(check_num_players(aux)==1){ 
				while(aux->next!=NULL){
					aux=aux->next;
					/*send message to clients to pause the game until further connection*/
					send(aux->socket_id, "Pause Game", 128*sizeof(char), 0);
					printf("\tOnly one player! Pause Game.\n");
				}
			}
			break;
		}
		else
			printf("Error: Client sent this data:\n %s", server_message);

	}
	pthread_exit(0);
}
void insert_play(int board_x, int board_y, int client_socket_id, play_response resp, int timer){

	/* Finds socketlist node of client */
	sockets_list *aux = head;
	while(aux->next != NULL && aux->next->socket_id != client_socket_id)
			aux = aux->next;
	aux = aux->next;
	printf("Play:(%d %d)\n\n", board_x, board_y);
	pthread_mutex_lock(&lock);		
	resp = board_play(aux, board_x, board_y, aux->play1);
	pthread_mutex_unlock(&lock);
	/* Case which card turns white again */
	if(timer == 1 && aux->first_play_time != 0){
		paint_card(board_x, board_y , 255, 255, 255);
		update_clients(client_socket_id, -1, board_x, board_y, -1, -1, resp.str_play1, resp.str_play2);
		aux->play1[0] = -1;
		aux->play1[1] = -1;
		return;
	}
	switch (resp.code) {
		case -2:
			printf("Case -2\n");		
			paint_card(resp.play1[0], resp.play1[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play1[0], resp.play1[1], resp.str_play1, 255, 0, 0);
			paint_card(resp.play2[0], resp.play2[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play2[0], resp.play2[1], resp.str_play2, 255, 0, 0);

			update_clients(client_socket_id, -2, resp.play1[0], resp.play1[1], resp.play2[0], resp.play2[1], resp.str_play1, resp.str_play2);
			board[linear_conv(resp.play1[0], resp.play1[1])].player_id = client_socket_id;
			board[linear_conv(resp.play2[0], resp.play2[1])].player_id = client_socket_id;
			sleep(5);
			board[linear_conv(resp.play1[0], resp.play1[1])].player_id = -1;
			board[linear_conv(resp.play2[0], resp.play2[1])].player_id = -1;
			paint_card(resp.play1[0], resp.play1[1] , 255, 255, 255);
			paint_card(resp.play2[0], resp.play2[1] , 255, 255, 255);
			update_clients(client_socket_id, -1, resp.play1[0], resp.play1[1], -1, -1, resp.str_play1, resp.str_play2);
			update_clients(client_socket_id, -1, resp.play2[0], resp.play2[1], -1, -1, resp.str_play1, resp.str_play2);
			aux->play1[0] = -1;
			aux->play1[1] = -1;
			break;
		case 1:
			printf("Case 1\n");
			paint_card(resp.play1[0], resp.play1[1] , 0.8*aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play1[0], resp.play1[1], resp.str_play1, 200, 200, 200);
			update_clients(client_socket_id, 1, resp.play1[0], resp.play1[1], -1, -1, resp.str_play1, resp.str_play2);
			/* Sets time for this 1st play */
			aux->first_play_time = (int)time(NULL);
			board[linear_conv(resp.play1[0], resp.play1[1])].player_id = client_socket_id;
			printf("Got 5 seconds to play\n");			
			

			break;
		case 2:
			printf("Case 2\n");
			paint_card(resp.play1[0], resp.play1[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
			paint_card(resp.play2[0], resp.play2[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
			update_clients(client_socket_id, 2, resp.play1[0], resp.play1[1], resp.play2[0], resp.play2[1], resp.str_play1, resp.str_play2);
			aux->play1[0] = -1;
			aux->play1[1] = -1;
			aux->first_play_time = 0;
			break;
		case 4:
			/* Reset */
			resp.code = -1;
			break;
		case 3:
			paint_card(resp.play1[0], resp.play1[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play1[0], resp.play1[1], resp.str_play1, 0, 0, 0);
			paint_card(resp.play2[0], resp.play2[1] , aux->rgb[0], aux->rgb[1], aux->rgb[2]);
			write_card(resp.play2[0], resp.play2[1], resp.str_play2, 0, 0, 0);
			update_clients(client_socket_id, 2, resp.play1[0], resp.play1[1], resp.play2[0], resp.play2[1], resp.str_play1, resp.str_play2);
			aux->play1[0] = -1;
			aux->play1[1] = -1;
			aux->first_play_time = 0;
			printf("Waiting 10 seconds to start a new game\n");
			sleep(10);
			update_clients(client_socket_id, 3, -1, -1, -1, -1, resp.str_play1, resp.str_play2);
			reset_start_new_game();
	}
}
void update_clients(int client_socket_id, int code, int play1_x, int play1_y, int play2_x, int play2_y, char *str_play1, char *str_play2){
	int server_commands[8]; 
	char server_message[128];
	int id_winner=0;
	char win[128];
	strcpy(server_message, "server_commands");
	/* Finds socketlist node of client */
	sockets_list *aux = head;
	while(aux->next != NULL && aux->next->socket_id != client_socket_id)
			aux = aux->next;
	aux = aux->next;

	/* Prepares info to send */
	/* Cards */
	server_commands[0] = code;
	server_commands[1] = play1_x;
	server_commands[2] = play1_y;
	server_commands[3] = play2_x;
	server_commands[4] = play2_y;
	server_commands[5] = aux->rgb[0];
	server_commands[6] = aux->rgb[1];
	server_commands[7] = aux->rgb[2];

	aux = head;
	if(head == NULL){
		printf("\n\n Error:Update clients, head=NULL \n");
		exit(EXIT_FAILURE);
	}else if(code == 4){ /* Code 4 means update this client only */

		while(aux->next != NULL && aux->next->socket_id != client_socket_id)
			aux = aux->next;
		if(aux->next == NULL)
			exit(EXIT_FAILURE);
		aux = aux->next;
		server_commands[0] = 2;
		while(send(aux->socket_id, &server_message, 128*sizeof(char), 0) == -1)
				continue;
		while(send(aux->socket_id, &server_commands, 8*sizeof(int), 0) == -1)
			continue;
		while(send(aux->socket_id, str_play1, 3*sizeof(char), 0) == -1)
			continue;
		while(send(aux->socket_id, str_play2, 3*sizeof(char), 0) == -1)
			continue;

	} else if(code==3){
		aux=head;
		   id_winner=check_winners(head);
		   while(aux->next!=NULL){
			   	aux=aux->next;

			   	if(aux->socket_id==id_winner){
			   		while(send(aux->socket_id, "Game Over!\n You are the winner!\n", 128*sizeof(char), 0)==-1)
			   			continue;
			   	}
			   	else{
				   	snprintf(win, 128*sizeof(char), "Game Over!\n The winner is player %d.\n", aux->socket_id);
				   	while(send(aux->socket_id, "Game Over!\n The winner is player %d.\n", 128*sizeof(char), 0)==-1)
				   		continue;
			   	}
			   	server_commands[0]=3;
			   	while(send(aux->socket_id, server_message, 128*sizeof(char), 0) == -1)
				continue;
				while(send(aux->socket_id, server_commands, 8*sizeof(int), 0) == -1)
					continue;
				while(send(aux->socket_id, str_play1, 3*sizeof(char), 0) == -1)
					continue;
				while(send(aux->socket_id, str_play2, 3*sizeof(char), 0) == -1)
					continue;
		   }
		   	printf("Winner %d\n", id_winner);
	}
	else{
		
		while(aux->next != NULL){
			aux = aux->next;
			printf("Sends server command\n");
			printf("socket id=%d\n", aux->socket_id);
			server_commands[0] = code;
			if(aux->socket_id == client_socket_id){
				if(server_commands[0] == -1){
					server_commands[0] = -3;
				}
				else if(server_commands[0] == -2){
					server_commands[0] = -4;
				}
			}
			while(send(aux->socket_id, server_message, 128*sizeof(char), 0) == -1)
				continue;
			while(send(aux->socket_id, server_commands, 8*sizeof(int), 0) == -1)
				continue;
			printf("STR1=%s\n", str_play1);
			while(send(aux->socket_id, str_play1, 3*sizeof(char), 0) == -1)
				continue;
			while(send(aux->socket_id, str_play2, 3*sizeof(char), 0) == -1)
				continue;
		}	
	}
}
void delete_player(int socket_id){
	sockets_list *aux = NULL;
	sockets_list *del = NULL;
	aux = head;

	while(aux->next != NULL && aux->next->socket_id != socket_id){
		aux = aux->next;
	}
	del = aux->next;
	aux->next = del->next;
	free(del);
	printf("Player with id = %d has left the game!\n", socket_id);
}
void close_clients(){
	sockets_list *aux = NULL;
	sockets_list *del = NULL;
	aux = head->next;
	/* In case all players have left, players memory are already free so we can just free the head and leave */
	if(head->next == NULL){
		free(head);
		return;
	}
	/* Free every player still in game and send him a message to die */
	while(aux->next != NULL){
		del = aux;
		aux = aux->next;
		while(send(del->socket_id, "die", 128*sizeof(char), 0) < 0)
				continue;
		free(del);
	}
	while(send(aux->socket_id, "die", 128*sizeof(char), 0) < 0)
				continue;
	free(aux);
	free(head);
	pthread_mutex_destroy(&lock);
}
int check_num_players(sockets_list *head){
	sockets_list *aux=NULL;
	int counter=0;
	aux=head;

	while(aux!=NULL){
		if(aux!=head){
			counter+=1;
		} 
		aux=aux->next;
	}

	printf("\t Counter=%d\n", counter);
	if (counter<=2)
		return 1; /*Pause Game*/

	return 0; 
}
void reset_start_new_game(){
	int i, j;

	
	/*reset needed variables and board*/
		for(i=0; i<dim_board; i++)
			for(j=0; j<dim_board; j++){
				board[linear_conv(i,j)].player_id = -1;
				paint_card(i, j , 255, 255, 255);
			}
	free(board);
	init_board(dim_board);
	return;
}
int check_winners(sockets_list *head){
	int counter=0;
	int i=0;
	int id;
	sockets_list *aux=NULL;
	aux=head;
	while(aux->next!=NULL){
		if(i==0){
			counter= aux->pairs;
			id=aux->socket_id;
		}
		if(aux->pairs>counter){
			counter=aux->pairs;
			id=aux->socket_id;
		}
		aux=aux->next;
	}

	return id;
}