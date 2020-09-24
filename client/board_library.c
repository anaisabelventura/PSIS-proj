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

#include "board_library.h"
#include "UI_library.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <pthread.h>

#include <poll.h>

int dim_board;
int play1[2];
int *done;

int linear_conv(int i, int j){
  return j*dim_board+i;
}


int init_board(int network_socket, int *done_main){
  done = done_main;
  while(recv(network_socket, &dim_board, sizeof(int), 0) < 0)
    continue;
  
  return dim_board;
}
void* server_messages(void* arg){
  char server_message[128];
  char str_play1[3];
  char str_play2[3];

  struct server_message_struct *arg_struct = (struct server_message_struct*) arg;

  while(1){
    while(recv(arg_struct->network_socket, &server_message, 128*sizeof(char), 0) < 0)
      continue;
    
    printf("SEVER MESSAGE:::::::%s\n", server_message);
    if(strcmp(server_message, "server_commands") == 0){
      printf("SEVER MESSAGE:::::::SEVRECOMMANDS\n");
      while(recv(arg_struct->network_socket, &arg_struct->server_commands, 8*sizeof(int), 0) < 0)
        continue;
      while(recv(arg_struct->network_socket, &str_play1, 3*sizeof(char), 0) < 0)
        continue;
      while(recv(arg_struct->network_socket, &str_play2, 3*sizeof(char), 0) < 0)
        continue;
      arg_struct->code = arg_struct->server_commands[0];
      if(arg_struct->server_commands[0] == -3 ){ /* Only unblocks clicks if -3 code is issued by the server to this specific client */
        arg_struct->click_block = 0;
        arg_struct->server_commands[0] = -1;
      }
      else if(arg_struct->server_commands[0] == -4 ){
        arg_struct->click_block = 1;
        arg_struct->server_commands[0] = -2;
      }
      printf("SERVERCOMMANDS[0] CODE=%d\n", arg_struct->server_commands[0]);
      paint(arg_struct->server_commands, str_play1, str_play2);
      if(arg_struct->server_commands[0]==3){
        printf("Reset Game\n");
        //sleep(10);
        printf("Wait 10 seconds to start a new game.\n");
        paint(arg_struct->server_commands, str_play1, str_play2);
      }
    }
    else if(strcmp(server_message, "Pause Game") == 0){
      printf("Pause Game, wait for another client to join\n");
      break;
    }
    else if(strcmp(server_message, "die") == 0){
      //Closes socket
      close(arg_struct->network_socket);
      //Sets done to 1 so main quits
      *done = 1;
      break;
    }

  }

  pthread_exit(0);
}
void paint(int * server_commands, char *str_play1, char *str_play2){
    int i, j;
    switch (server_commands[0]) {
      case -1:
        paint_card(server_commands[1], server_commands[2] , 255, 255, 255);
        break;
      case 1:
        paint_card(server_commands[1], server_commands[2] , 0.8*server_commands[5], server_commands[6], server_commands[7]);
        write_card(server_commands[1], server_commands[2], str_play1, 200, 200, 200);
        break;
      case 2:
        printf("Correct\n"); 
        paint_card(server_commands[1], server_commands[2] , server_commands[5], server_commands[6], server_commands[7]);
        printf("%s\n", str_play1);
        printf("paintcard(%d,%d,%d,%d,%d)\n", server_commands[1], server_commands[2] , server_commands[5], server_commands[6], server_commands[7]);
        write_card(server_commands[1], server_commands[2], str_play1, 0, 0, 0);
        paint_card(server_commands[3], server_commands[4] , server_commands[5], server_commands[6], server_commands[7]);
        printf("paintcard(%d,%d,%d,%d,%d)\n", server_commands[3], server_commands[4] , server_commands[5], server_commands[6], server_commands[7]);
        printf("%s\n", str_play2);
        write_card(server_commands[3], server_commands[4], str_play2, 0, 0, 0);
        break;
      case -2: // Clicked in two different cards
        paint_card(server_commands[1], server_commands[2] , server_commands[5], server_commands[6], server_commands[7]);
        write_card(server_commands[1], server_commands[2], str_play1, 255, 0, 0);
        paint_card(server_commands[3], server_commands[4] , server_commands[5], server_commands[6], server_commands[7]);
        write_card(server_commands[3], server_commands[4], str_play2, 255, 0, 0);
        break;
      case 3:
        for (i = 0; i < dim_board; i++)
          for(j=0; j<dim_board; j++)
            paint_card(i, j, 255, 255,255);
        break;
    }
}
