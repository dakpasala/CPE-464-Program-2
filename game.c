/*****************************************************************************
 * game.c - Game state management implementation
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_GAMES 100

/* Game state structure */
typedef struct {
    int active;           /* 1 if game is active, 0 if slot is free */
    int x_socket;
    int o_socket;
    uint8_t board[9];     /* 0=empty, 1=X, 2=O */
    int current_turn;     /* SYMBOL_X or SYMBOL_O */
} Game;

/* Array of games */
static Game games[MAX_GAMES];

/* Win conditions: 8 possible ways to win */
static const int win_patterns[8][3] = {
    {0, 1, 2},  /* Row 1 */
    {3, 4, 5},  /* Row 2 */
    {6, 7, 8},  /* Row 3 */
    {0, 3, 6},  /* Col 1 */
    {1, 4, 7},  /* Col 2 */
    {2, 5, 8},  /* Col 3 */
    {0, 4, 8},  /* Diagonal \ */
    {2, 4, 6}   /* Diagonal / */
};

/*****************************************************************************
 * game_init - Initialize the game management system
 *****************************************************************************/
void game_init(void) {
    memset(games, 0, sizeof(games));
}

/*****************************************************************************
 * game_create - Create a new game between two players
 *****************************************************************************/
int game_create(int x_socket, int o_socket) {
    int i;

    /* Find a free slot */
    for (i = 0; i < MAX_GAMES; i++) {
        if (!games[i].active) {
            games[i].active = 1;
            games[i].x_socket = x_socket;
            games[i].o_socket = o_socket;
            memset(games[i].board, CELL_EMPTY, 9);
            games[i].current_turn = SYMBOL_X;  /* X goes first */
            return i;
        }
    }

    return -1;  /* No free slots */
}

/*****************************************************************************
 * game_get_by_socket - Get game ID for a player's socket
 *****************************************************************************/
int game_get_by_socket(int socket) {
    int i;

    for (i = 0; i < MAX_GAMES; i++) {
        if (games[i].active &&
            (games[i].x_socket == socket || games[i].o_socket == socket)) {
            return i;
        }
    }

    return -1;
}

/*****************************************************************************
 * game_get_opponent - Get the opponent's socket
 *****************************************************************************/
int game_get_opponent(int game_id, int socket) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    if (games[game_id].x_socket == socket) {
        return games[game_id].o_socket;
    } else if (games[game_id].o_socket == socket) {
        return games[game_id].x_socket;
    }

    return -1;
}

/*****************************************************************************
 * game_get_symbol - Get the symbol (X or O) for a socket in a game
 *****************************************************************************/
int game_get_symbol(int game_id, int socket) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    if (games[game_id].x_socket == socket) {
        return SYMBOL_X;
    } else if (games[game_id].o_socket == socket) {
        return SYMBOL_O;
    }

    return -1;
}

/*****************************************************************************
 * game_is_turn - Check if it's a player's turn
 *****************************************************************************/
int game_is_turn(int game_id, int socket) {
    int symbol;

    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return 0;
    }

    symbol = game_get_symbol(game_id, socket);
    if (symbol < 0) {
        return 0;
    }

    return (games[game_id].current_turn == symbol);
}

/*****************************************************************************
 * game_make_move - Make a move on the board
 *****************************************************************************/
int game_make_move(int game_id, int socket, int position) {
    int symbol;
    int board_index;

    /* Validate game ID */
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    /* Validate position (1-9) */
    if (position < 1 || position > 9) {
        return -3;
    }

    /* Convert to 0-based index */
    board_index = position - 1;

    /* Check if it's this player's turn */
    symbol = game_get_symbol(game_id, socket);
    if (symbol < 0 || games[game_id].current_turn != symbol) {
        return -2;
    }

    /* Check if position is empty */
    if (games[game_id].board[board_index] != CELL_EMPTY) {
        return -4;
    }

    /* Make the move */
    games[game_id].board[board_index] = (symbol == SYMBOL_X) ? CELL_X : CELL_O;

    /* Switch turns */
    games[game_id].current_turn = (symbol == SYMBOL_X) ? SYMBOL_O : SYMBOL_X;

    return 0;
}

/*****************************************************************************
 * game_get_board - Get the current board state
 *****************************************************************************/
int game_get_board(int game_id, uint8_t *board) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    memcpy(board, games[game_id].board, 9);
    return 0;
}

/*****************************************************************************
 * game_get_current_turn - Get whose turn it is
 *****************************************************************************/
int game_get_current_turn(int game_id) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    return games[game_id].current_turn;
}

/*****************************************************************************
 * game_check_winner - Check if there's a winner
 *****************************************************************************/
int game_check_winner(int game_id) {
    int i;
    uint8_t *board;

    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    board = games[game_id].board;

    /* Check all 8 win patterns */
    for (i = 0; i < 8; i++) {
        int pos1 = win_patterns[i][0];
        int pos2 = win_patterns[i][1];
        int pos3 = win_patterns[i][2];

        if (board[pos1] != CELL_EMPTY &&
            board[pos1] == board[pos2] &&
            board[pos2] == board[pos3]) {
            return board[pos1];  /* Return CELL_X or CELL_O */
        }
    }

    return CELL_EMPTY;  /* No winner */
}

/*****************************************************************************
 * game_is_draw - Check if the game is a draw
 *****************************************************************************/
int game_is_draw(int game_id) {
    int i;
    uint8_t *board;

    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return 0;
    }

    /* First check if there's a winner */
    if (game_check_winner(game_id) != CELL_EMPTY) {
        return 0;  /* There's a winner, not a draw */
    }

    board = games[game_id].board;

    /* Check if board is full */
    for (i = 0; i < 9; i++) {
        if (board[i] == CELL_EMPTY) {
            return 0;  /* Board not full */
        }
    }

    return 1;  /* Board full, no winner = draw */
}

/*****************************************************************************
 * game_destroy - Remove a game
 *****************************************************************************/
int game_destroy(int game_id) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    games[game_id].active = 0;
    return 0;
}

/*****************************************************************************
 * game_destroy_by_socket - Remove any game involving a socket
 *****************************************************************************/
int game_destroy_by_socket(int socket) {
    int game_id = game_get_by_socket(socket);

    if (game_id < 0) {
        return -1;
    }

    return game_destroy(game_id);
}

/*****************************************************************************
 * game_get_x_socket - Get X player's socket
 *****************************************************************************/
int game_get_x_socket(int game_id) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    return games[game_id].x_socket;
}

/*****************************************************************************
 * game_get_o_socket - Get O player's socket
 *****************************************************************************/
int game_get_o_socket(int game_id) {
    if (game_id < 0 || game_id >= MAX_GAMES || !games[game_id].active) {
        return -1;
    }

    return games[game_id].o_socket;
}

/*****************************************************************************
 * game_cleanup - Free all memory used by game management
 *****************************************************************************/
void game_cleanup(void) {
    memset(games, 0, sizeof(games));
}
