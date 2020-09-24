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
#include <pthread.h>
#include <time.h>

typedef struct board_place{
  char v[3];
  int player_id;
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
	int got_board;
	int rgb[3];
	pthread_t tid;
	play_response resp;
	struct sockets_list *next;
    int first_play_time;
    int play1[2];
    int pairs;
} sockets_list;

struct server_wait_conn_struct {
	sockets_list* head;
};

typedef struct client_thread_struct {
  sockets_list** client;
  sockets_list** head;
} client_thread_struct;

char * get_board_place_str(int i, int j);
void init_board(int dim);
play_response board_play(sockets_list *client, int x, int y, int play1[]);
void* server_wait_conn(void* arg);
void get_initial_screen(int socket_id);
void* client_thread(void* arg);
void delete_player(int socket_id);
void insert_play(int board_x, int board_y, int client_socket_id, play_response resp, int timer);
void* receive_play(void* arg);
void update_clients(int player_id, int code, int play1_x, int play1_y, int play2_x, int play2_y, char *str_play1, char *str_play2);
void close_clients();
int check_num_players(sockets_list *head);
int check_pairs();
int check_winners(sockets_list *head);
void reset_start_new_game();
#endif
