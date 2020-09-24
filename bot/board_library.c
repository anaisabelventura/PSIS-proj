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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>


int dim_board=4;

board_place * board;
int play1[2];
int *done;

int linear_conv(int i, int j){
  return j*dim_board+i;
}
char * get_board_place_str(int i, int j){
  return board[linear_conv(i, j)].v;
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
    if(strcmp(server_message, "server_commands") == 0){
      while(recv(arg_struct->network_socket, &arg_struct->server_commands, 8*sizeof(int), 0) < 0)
        continue;
      while(recv(arg_struct->network_socket, &str_play1, 3*sizeof(char), 0) < 0)
        continue;
      while(recv(arg_struct->network_socket, &str_play2, 3*sizeof(char), 0) < 0)
        continue;
       if(arg_struct->bot==NULL)
        printf("Bot error - pointer is null.\n");
      arg_struct->bot=bot_client_play(arg_struct->network_socket, arg_struct->bot, arg_struct->server_commands[1], arg_struct->server_commands[2], arg_struct->server_commands[3], arg_struct->server_commands[4],str_play2,str_play1);
      sleep(2);
      arg_struct->code = arg_struct->server_commands[0];
    }else if (strcmp(server_message, "die") == 0){
      *done = 1;
      break;
    }
  }

  pthread_exit(0);
}

void die(){
  free(board);
}

bot_client *init_bot(){

  int i=0, j=0;
  /*allcoc memory for arrays*/
  struct bot_client *bot=(bot_client *)malloc(sizeof(bot_client)); /*struct*/
  if (bot==NULL){
    printf("\tError 1 in bot allocation\n");
    exit(1);
  }
  /*Matrix for board*/
  bot->pick1= (card_info *)malloc(sizeof(card_info));
  if (bot->pick1==NULL){
    printf("\tError 2 in bot allocation\n");
    exit(1);
  }
  bot->pick2= (card_info *)malloc(sizeof(card_info));
  if (bot->pick2==NULL){
    printf("\tError 2.1 in bot allocation\n");
    exit(1);
  }

  bot->board=(card_info **)malloc(sizeof(card_info *)*dim_board); /*1st column*/
  for(i=0; i<dim_board; i++)
    bot->board[i] = (card_info *)malloc(sizeof(card_info) * dim_board); /*remaining columnns*/

  if (bot->board==NULL){
    printf("\tError 3 in bot allocation\n");
    exit(1);
  }
  /*init struct variables*/
  for(i=0; i<dim_board; i++)
    for(j=0; j<dim_board; j++){
      bot->board[i][j].flag=-1;  /*-1 if card hasn't been shown; 
                                  0 if card is known but not yet in a pair;
                                  1 if card is in a pair*/
      bot->board[i][j].coord[0]=i;
      bot->board[i][j].coord[1]=j;
      bot->board[i][j].card[0]='.';
      bot->board[i][j].card[1]='.';      
      bot->board[i][j].card[3]='\0';
    }
    bot->pick2->coord[0]=0;
    bot->pick1->coord[0]=0;
    bot->pick2->coord[1]=0;
    bot->pick1->coord[1]=0;
    bot->pairs=0;

  return bot;
}

bot_client *bot_client_play(int network_socket, bot_client *bot, int play1_x, int play1_y, int play2_x, int play2_y, char str_play2[3], char str_play1[3]){
  
  int i,j, k,h, e, f, m, n;
  char server_message[128];
  const char *ver="..";
  if(bot==NULL){
    printf("\tError\n");
    exit(1);
  }
  
  strcpy(server_message, "coordinates");
  /*Checks if card isn't already known*/
  if((play1_x >=0 && play1_x<dim_board)&&(play1_y>=0 && play1_y<dim_board)){
    if((bot->board[play1_y][play1_x]).flag==0){
      /*YES -> sees if there is a matching card*/
          /*YES -> picks both cards and forms pair*/
          /*NO -> moves on to play 2*/
        for(i=0; i<dim_board; i++){
              for(j=0; j<dim_board; j++){
                if(strcmp(str_play1, (bot->board[i][j]).card)==0 && (j!=play1_x || i!=play1_y) && strcmp(ver,bot->board[i][j].card)!=0){
                  bot->pick2->coord[0]=i;
                  bot->pick2->coord[1]=j;
                  bot->board[i][j].flag=1;
                  send(network_socket, &server_message, 128*sizeof(char), 0);
                  send(network_socket, bot->pick2->coord, 2*sizeof(int), 0);
                  bot->pick1->coord[0]=play1_y;
                  bot->pick1->coord[1]=play1_x;
                  send(network_socket, &server_message, 128*sizeof(char), 0);
                  send(network_socket, bot->pick1->coord, 2*sizeof(int), 0);
                  bot->board[play1_y][play1_x].flag=1;
                  bot->pairs+=1;
                  printf("c -pair %d %d e %d %d\n", play1_y, play1_x, i,j);
                }
                break;
              }
            break;
          }
    } else if(bot->board[play1_y][play1_x].flag==-1){ /*card is unknown, see if there is a pair or save it*/
        /*see if there is a matching card*/
        bot->board[play1_y][play1_x].flag=0;
        strcpy( bot->board[play1_y][play1_x].card, str_play1);
        printf("save card %d %d %c%c\n",play1_y, play1_x, bot->board[play1_y][play1_x].card[0], bot->board[play1_y][play1_x].card[1] );
        
        for(m=0; m<dim_board; m++){
          for( n=0; n<dim_board; n++){
            if(strcmp(str_play1, bot->board[m][n].card)==0 && (n!=play1_x || m!=play1_y) && strcmp(ver,bot->board[m][n].card)!=0){
              bot->pick2->coord[0]=m;
              bot->pick2->coord[1]=n;
              bot->board[m][n].flag=1;
              send(network_socket, &server_message, 128*sizeof(char), 0);
              send(network_socket, bot->pick2->coord, 2*sizeof(int), 0);
              bot->pick1->coord[0]=play1_y;
              bot->pick1->coord[1]=play1_x;
              send(network_socket, &server_message, 128*sizeof(char), 0);
              send(network_socket, bot->pick1->coord, 2*sizeof(int), 0);
              bot->board[play1_y][play1_x].flag=1;
              printf("Pair %d %d e %d %d\n", m,n,play1_y, play1_x);
            }
            break;
          }
          break; 
        }    
    } 
  }
   /*Checks if card isn't already known*/
  if((play2_x >=0 && play2_x<dim_board)&&(play2_y>=0 && play2_y<dim_board)){
    if(bot->board[play2_y][play2_x].flag==0){
      /*YES -> sees if there is a matching card*/
          /*YES -> picks both cards and forms pair*/
          /*NO -> moves on to play 2*/
        for(k=0; k<dim_board; k++){
              for(h=0; h<dim_board; h++){
                if(strcmp(str_play2,  (bot->board[k][h]).card)==0 && (h!=play2_x || k!=play2_y) && strcmp(ver,bot->board[k][h].card)!=0){
                  bot->pick2->coord[0]=k;
                  bot->pick2->coord[1]=h;
                  bot->board[k][h].flag=1;
                  send(network_socket, &server_message, 128*sizeof(char), 0);
                  send(network_socket, bot->pick2->coord, 2*sizeof(int), 0);
                  bot->pick1->coord[0]=play2_y;
                  bot->pick1->coord[1]=play2_x;
                  send(network_socket, &server_message, 128*sizeof(char), 0);
                  send(network_socket, bot->pick1->coord, 2*sizeof(int), 0);
                  bot->board[play2_y][play2_x].flag=1;
                  printf("pair %d %d e %d %d\n", play2_y, play2_x, k,h);
                }
                break;
              }
            break;
          }
    }else if(bot->board[play2_y][play2_x].flag==-1){ /*card is unknown, see if there is a pair or save it*/
        /*see if there is a matching card*/
        bot->board[play2_y][play2_x].flag=0;
        strcpy( bot->board[play2_y][play2_x].card, str_play2);
        printf("save card %d %d %c%c\n",play2_y, play1_x, bot->board[play2_y][play2_x].card[0], bot->board[play2_y][play2_x].card[1]);
        
     
        for(e=0; e<dim_board; e++){
          for(f=0; f<dim_board; f++){
            if(strcmp(str_play2, bot->board[e][f].card)==0 && (f!=play2_x || e!=play2_y) && strcmp(ver,bot->board[e][f].card)!=0){
              bot->pick2->coord[0]=e;
              bot->pick2->coord[1]=f;
              bot->board[e][f].flag=1;
              send(network_socket, &server_message, 128*sizeof(char), 0);
              send(network_socket, bot->pick2->coord, 2*sizeof(int), 0);
              bot->pick1->coord[0]=play2_y;
              bot->pick1->coord[1]=play2_x;
              send(network_socket, &server_message, 128*sizeof(char), 0);
              send(network_socket, bot->pick1->coord, 2*sizeof(int), 0);
              bot->board[play2_y][play2_x].flag=1;
             printf("Pair: %d %d e %d %d\n", play2_y, play2_x, e,f);

            }
          break;
          }
        break;
        }     
    } 
  }
    
  return bot;
}
int check_win(int network_socket, bot_client *bot){

  int i, j;
  int counter=0;

  for(i=0; i<dim_board; i++)
    for(j=0; j<dim_board; j++)
      if(bot->board[i][j].flag==1)
        counter+=1;
    if(counter==(dim_board*dim_board))
      return 1;
  return 0;
}