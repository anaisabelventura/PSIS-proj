/******************************************************************************
 * File Name: board_library.h
 *        (c) 2019 PSIS
 * Authors:    Ana Isabel Ventura, Tiago Marques
 * Last modified: 31/05/2019
 *
 * COMMENTS
 *    
 *****************************************************************************/
#ifndef _BOARD_LIBRARY_H
#define _BOARD_LIBRARY_H

#include <stdlib.h>

typedef struct board_place{
  char v[3];
} board_place;

typedef struct play_response{
  int code; // 0 - filled
            // 1 - 1st play
            // 2 2nd - same plays
            // 3 END
            // -2 2nd - diffrent
  int play1[2];
  int play2[2];
  char str_play1[3], str_play2[3];
} play_response;

typedef struct coordinates{
  int board_x;
  int board_y;
} coordinates;

typedef struct sockets_list {
	int socket_id;
	int player_id;
  struct sockets_list *next;
} sockets_list;

struct server_message_struct {
  int code;            /* [0]-unpaint a card [1]- paint card [2]-lockcards */
  int server_commands[8];   /* [0]-board_x   [1]-board_y  [2][3] - board [4][5][6]-colors */
  int network_socket;
  int click_block;
};

char * get_board_place_str(int i, int j);
int init_board(int dim, int *done_main);
void* server_messages(void* arg);
void send_board(sockets_list *head);
void paint(int * server_commands, char *str_play1, char *str_play2);

#endif