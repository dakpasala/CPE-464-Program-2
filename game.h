/*****************************************************************************
 * game.h - Game state management (server-side)
 *
 * This module manages active tic-tac-toe games, including board state,
 * turn tracking, move validation, and win/draw detection.
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* Board cell values */
#define CELL_EMPTY  0
#define CELL_X      1
#define CELL_O      2

/* Player symbols */
#define SYMBOL_O    0
#define SYMBOL_X    1

/* Game result codes */
#define RESULT_DRAW       0
#define RESULT_X_WON      1
#define RESULT_O_WON      2
#define RESULT_X_FORFEIT  3
#define RESULT_O_FORFEIT  4
#define RESULT_X_DISCONN  5
#define RESULT_O_DISCONN  6

/*****************************************************************************
 * game_init - Initialize the game management system
 *****************************************************************************/
void game_init(void);

/*****************************************************************************
 * game_create - Create a new game between two players
 *
 * Parameters:
 *   x_socket  - Socket of player X (challenger)
 *   o_socket  - Socket of player O (challenged)
 *
 * Returns:
 *   Game ID (>= 0) on success
 *   -1 on error (memory allocation failure or too many games)
 *****************************************************************************/
int game_create(int x_socket, int o_socket);

/*****************************************************************************
 * game_get_by_socket - Get game ID for a player's socket
 *
 * Parameters:
 *   socket - The socket to search for
 *
 * Returns:
 *   Game ID (>= 0) if player is in a game
 *   -1 if player is not in any game
 *****************************************************************************/
int game_get_by_socket(int socket);

/*****************************************************************************
 * game_get_opponent - Get the opponent's socket
 *
 * Parameters:
 *   game_id - The game ID
 *   socket  - The socket of one player
 *
 * Returns:
 *   Opponent's socket on success
 *   -1 on error (game not found or socket not in game)
 *****************************************************************************/
int game_get_opponent(int game_id, int socket);

/*****************************************************************************
 * game_get_symbol - Get the symbol (X or O) for a socket in a game
 *
 * Parameters:
 *   game_id - The game ID
 *   socket  - The socket to check
 *
 * Returns:
 *   SYMBOL_X (1) or SYMBOL_O (0)
 *   -1 on error (game not found or socket not in game)
 *****************************************************************************/
int game_get_symbol(int game_id, int socket);

/*****************************************************************************
 * game_is_turn - Check if it's a player's turn
 *
 * Parameters:
 *   game_id - The game ID
 *   socket  - The socket to check
 *
 * Returns:
 *   1 if it's this player's turn
 *   0 if it's not their turn or on error
 *****************************************************************************/
int game_is_turn(int game_id, int socket);

/*****************************************************************************
 * game_make_move - Make a move on the board
 *
 * Parameters:
 *   game_id  - The game ID
 *   socket   - The socket making the move
 *   position - Position 1-9
 *
 * Returns:
 *   0 on success
 *   -1 if game not found
 *   -2 if not player's turn
 *   -3 if position invalid (not 1-9)
 *   -4 if position already occupied
 *****************************************************************************/
int game_make_move(int game_id, int socket, int position);

/*****************************************************************************
 * game_get_board - Get the current board state
 *
 * Parameters:
 *   game_id - The game ID
 *   board   - Buffer to store board (must be at least 9 bytes)
 *
 * Returns:
 *   0 on success
 *   -1 if game not found
 *
 * Note: board[0] = position 1, board[1] = position 2, ..., board[8] = position 9
 *****************************************************************************/
int game_get_board(int game_id, uint8_t *board);

/*****************************************************************************
 * game_get_current_turn - Get whose turn it is
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   SYMBOL_X or SYMBOL_O
 *   -1 if game not found
 *****************************************************************************/
int game_get_current_turn(int game_id);

/*****************************************************************************
 * game_check_winner - Check if there's a winner
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   CELL_X (1) if X won
 *   CELL_O (2) if O won
 *   CELL_EMPTY (0) if no winner yet
 *   -1 if game not found
 *****************************************************************************/
int game_check_winner(int game_id);

/*****************************************************************************
 * game_is_draw - Check if the game is a draw
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   1 if board is full with no winner (draw)
 *   0 if not a draw or game not found
 *****************************************************************************/
int game_is_draw(int game_id);

/*****************************************************************************
 * game_destroy - Remove a game
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   0 on success
 *   -1 if game not found
 *****************************************************************************/
int game_destroy(int game_id);

/*****************************************************************************
 * game_destroy_by_socket - Remove any game involving a socket
 *
 * Parameters:
 *   socket - The socket descriptor
 *
 * Returns:
 *   0 if a game was destroyed
 *   -1 if no game found for this socket
 *****************************************************************************/
int game_destroy_by_socket(int socket);

/*****************************************************************************
 * game_get_x_socket - Get X player's socket
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   X player's socket on success
 *   -1 if game not found
 *****************************************************************************/
int game_get_x_socket(int game_id);

/*****************************************************************************
 * game_get_o_socket - Get O player's socket
 *
 * Parameters:
 *   game_id - The game ID
 *
 * Returns:
 *   O player's socket on success
 *   -1 if game not found
 *****************************************************************************/
int game_get_o_socket(int game_id);

/*****************************************************************************
 * game_cleanup - Free all memory used by game management
 *****************************************************************************/
void game_cleanup(void);

#endif /* GAME_H */
