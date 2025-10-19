#define _POSIX_C_SOURCE 200809L

/*****************************************************************************
 * client.c - Tic-Tac-Toe client implementation
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <strings.h>

#include "pdu.h"

/* Packet flags */
#define FLAG_INITIAL_CONN      1
#define FLAG_CONN_ACCEPT       2
#define FLAG_CONN_REJECT       3
#define FLAG_ERROR             7
#define FLAG_LIST_REQ          10
#define FLAG_LIST_COUNT        11
#define FLAG_LIST_USER         12
#define FLAG_LIST_DONE         13
#define FLAG_GAME_START_REQ    20
#define FLAG_GAME_STARTED      21
#define FLAG_GAME_START_ERR    22
#define FLAG_MOVE              30
#define FLAG_BOARD_UPDATE      31
#define FLAG_MOVE_INVALID      32
#define FLAG_GAME_OVER         33

#define BUFFER_SIZE 2048
#define MAX_INPUT 1400

/* ANSI color codes */
#define COLOR_RED     "\033[91m"
#define COLOR_BLUE    "\033[94m"
#define COLOR_CYAN    "\033[96m"
#define COLOR_YELLOW  "\033[93m"
#define COLOR_RESET   "\033[0m"

/* Client state */
typedef enum {
    STATE_AVAILABLE,
    STATE_IN_GAME
} ClientState;

/* Global state */
static ClientState client_state = STATE_AVAILABLE;
static char my_username[101];
static int current_game_id = -1;
static int my_symbol = -1;  /* 0=O, 1=X */
static uint8_t board[9];

/* Function prototypes */
int connect_to_server(const char *hostname, uint16_t port);
int send_initial_connection(int socket, const char *username);
void run_client(int socket);
void handle_stdin(int socket);
void handle_server_data(int socket);
void process_command(int socket, const char *input);
void send_list_request(int socket);
void send_game_start_request(int socket, const char *opponent);
void send_move(int socket, int position);
void handle_conn_accept(void);
void handle_conn_reject(uint8_t *buffer, int len);
void handle_list_response(int socket, uint8_t *initial_buffer, int initial_len);
void handle_game_started(uint8_t *buffer, int len);
void handle_game_start_error(uint8_t *buffer, int len);
void handle_board_update(uint8_t *buffer, int len);
void handle_move_invalid(uint8_t *buffer, int len);
void handle_game_over(uint8_t *buffer, int len);
void display_board(void);
void init_board(void);

/*****************************************************************************
 * main - Entry point
 *****************************************************************************/
int main(int argc, char *argv[]) {
    int socket_fd;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s username hostname port\n", argv[0]);
        exit(1);
    }

    if (strlen(argv[1]) > 100) {
        fprintf(stderr, "Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
        exit(1);
    }

    strcpy(my_username, argv[1]);
    socket_fd = connect_to_server(argv[2], (uint16_t)atoi(argv[3]));

    if (send_initial_connection(socket_fd, argv[1]) < 0) {
        close(socket_fd);
        exit(1);
    }

    run_client(socket_fd);
    close(socket_fd);
    return 0;
}

/*****************************************************************************
 * TODO: connect_to_server
 *
 * STUDENT IMPLEMENTATION REQUIRED
 *
 * Create a TCP socket and connect to the server.
 *
 * Steps:
 *   1. Use getaddrinfo() to resolve the hostname and port
 *      - Set hints.ai_family = AF_INET (IPv4)
 *      - Set hints.ai_socktype = SOCK_STREAM (TCP)
 *   2. Try each address returned by getaddrinfo()
 *      - Create a socket with socket()
 *      - Try to connect() to the address
 *      - If connect succeeds, break; otherwise close and try next
 *   3. Don't forget to freeaddrinfo()
 *   4. Return the connected socket descriptor
 *   5. If connection fails, print error and exit(1)
 *
 * Return: Connected socket file descriptor
 *****************************************************************************/
int connect_to_server(const char *hostname, uint16_t port) {
    /* TODO: Implement socket creation and connection to server */

    fprintf(stderr, "ERROR: connect_to_server() not yet implemented\n");
    exit(1);
}

/*****************************************************************************
 * send_initial_connection - Send Flag 1 and process response (COMPLETE)
 *
 * This is a COMPLETE EXAMPLE showing how to:
 *   1. BUILD a packet with: flag + length + data
 *   2. SEND it using sendPDU()
 *   3. RECEIVE a response using recvPDU()
 *   4. PARSE the response and dispatch to handlers
 *
 * Flag 1 Packet Format:
 *   +------+-------------+------------------+
 *   | Flag | Username    | Username         |
 *   |      | Length      | (variable)       |
 *   +------+-------------+------------------+
 *   | 1 B  | 1 B         | username_len B   |
 *   +------+-------------+------------------+
 *
 * Example for username "alice" (5 characters):
 *   buffer[0] = 1       (FLAG_INITIAL_CONN)
 *   buffer[1] = 5       (length of "alice")
 *   buffer[2] = 'a'
 *   buffer[3] = 'l'
 *   buffer[4] = 'i'
 *   buffer[5] = 'c'
 *   buffer[6] = 'e'
 *   Total packet size: 1 + 1 + 5 = 7 bytes
 *
 * Response packets:
 *   - Flag 2 (CONN_ACCEPT): Just flag, 1 byte
 *   - Flag 3 (CONN_REJECT): flag + length + username
 *
 * Return: 0 on success, -1 on failure
 *****************************************************************************/
int send_initial_connection(int socket, const char *username) {
    uint8_t send_buffer[BUFFER_SIZE];
    uint8_t recv_buffer[BUFFER_SIZE];
    uint8_t username_len;
    int bytes_received;

    /* Step 1: Build the packet */
    username_len = strlen(username);
    send_buffer[0] = FLAG_INITIAL_CONN;      /* Flag */
    send_buffer[1] = username_len;           /* Length */
    memcpy(send_buffer + 2, username, username_len);  /* Data */

    /* Step 2: Send the packet */
    sendPDU(socket, send_buffer, 2 + username_len);

    /* Step 3: Receive the response */
    bytes_received = recvPDU(socket, recv_buffer, BUFFER_SIZE);
    if (bytes_received <= 0) {
        fprintf(stderr, "Server terminated connection\n");
        return -1;
    }

    /* Step 4: Parse the response flag and dispatch */
    uint8_t response_flag = recv_buffer[0];

    if (response_flag == FLAG_CONN_ACCEPT) {
        handle_conn_accept();
        return 0;
    } else if (response_flag == FLAG_CONN_REJECT) {
        handle_conn_reject(recv_buffer, bytes_received);
        return -1;
    } else {
        fprintf(stderr, "Unexpected response from server\n");
        return -1;
    }
}

/*****************************************************************************
 * TODO: run_client
 *
 * STUDENT IMPLEMENTATION REQUIRED
 *
 * Main client loop using poll() to monitor stdin and server socket.
 *
 * Steps:
 *   1. Initialize the board with init_board()
 *
 *   2. Set up poll() with two file descriptors:
 *      - pfds[0].fd = STDIN_FILENO, pfds[0].events = POLLIN
 *      - pfds[1].fd = socket, pfds[1].events = POLLIN
 *
 *   3. Loop forever:
 *      - Call poll(pfds, 2, -1) to wait for events
 *      - If pfds[0].revents & POLLIN: call handle_stdin(socket)
 *      - If pfds[1].revents & POLLIN: call handle_server_data(socket)
 *
 * Note: This function never returns normally (exits on server disconnect)
 *****************************************************************************/
void run_client(int socket) {
    /* TODO: Implement poll() loop monitoring stdin and server socket */

    init_board();
    struct pollfd pfds[2];
    pfds[0]fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = socket;
    pfds[1].events = POLLIN;
    while (1) {
        int events = poll(pfds, 2, -1);
        if (events < 0) continue;
        if (pfds[0].revents && POLLIN) handle_stdin(socket);
        if (pfds[1].revents && POLLIN) handle_server_data(socket);
    }

}

/*****************************************************************************
 * TODO: handle_stdin
 *
 * STUDENT IMPLEMENTATION REQUIRED
 *
 * Read user input from stdin and process commands.
 *
 * Steps:
 *   1. Print prompt "$: " only if not in a game (check client_state)
 *
 *   2. Read a line from stdin using fgets()
 *      - If fgets returns NULL, just return
 *
 *   3. Remove the newline character from the input
 *      - Use strcspn() or similar
 *
 *   4. Call process_command(socket, input) to parse and execute
 *
 * Note: The prompt should only appear when user is not in a game,
 *       or when it's their turn to move.
 *****************************************************************************/
void handle_stdin(int socket) {
    /* TODO: Implement reading from stdin and calling process_command() */

    char input[MAX_INPUT];

    if (client_state == STATE_AVAILABLE) {
        printf("$: ");
        fflush(stdout);
    }

    if (fgets(input, MAX_INPUT, stdin) == NULL) return;

    input[strcspn(input, "\n")] = '\0';
    process_command(socket, input);

}

/*****************************************************************************
 * TODO: handle_server_data
 *
 * STUDENT IMPLEMENTATION REQUIRED
 *
 * Receive and dispatch packets from the server.
 *
 * Steps:
 *   1. Receive a packet using recvPDU(socket, buffer, BUFFER_SIZE)
 *
 *   2. Check for errors:
 *      - If bytes_received == 0: server disconnected, exit(0)
 *      - If bytes_received < 0: error, exit(1)
 *
 *   3. Extract the flag from buffer[0]
 *
 *   4. Use a switch statement to dispatch based on flag:
 *      - FLAG_LIST_COUNT (11): call handle_list_response()
 *      - FLAG_GAME_STARTED (21): call handle_game_started()
 *      - FLAG_GAME_START_ERR (22): call handle_game_start_error()
 *      - FLAG_BOARD_UPDATE (31): call handle_board_update()
 *      - FLAG_MOVE_INVALID (32): call handle_move_invalid()
 *      - FLAG_GAME_OVER (33): call handle_game_over()
 *      - FLAG_ERROR (7): print generic error message from buffer+1
 *      - default: print "Unknown flag"
 *****************************************************************************/
void handle_server_data(int socket) {
    /* TODO: Implement receiving packets and dispatching to handlers */

    uint8_t buffer[BUFFER_SIZE];
    int bytes_recieved = recvPDU(socket, buffer, BUFFER_SIZE);

    if (bytes_recieved == 0) exit(0);
    if (bytes_recieved < 0) exit(1);

    uint8_t flag = buffer[0];
    switch (flag) {
        case 11:
            handle_list_response(socket, buffer, bytes_recieved);
            break;
        case 21:
            handle_game_started(buffer, bytes_recieved);
            break;
        case 22:
            handle_game_start_error(buffer, bytes_recieved);
            break;
        case 31:
            handle_board_update(buffer, bytes_recieved);
            break;
        case 32:
            handle_move_invalid(buffer, bytes_recieved);
            break;
        case 33:
            handle_game_over(buffer, bytes_recieved);
            break;
        case 7:
            printf("Error: %s\n", buffer + 1);
            break;
        default:
            printf("Unknown flag: %d\n", flag);
            break;
    }

}

/*****************************************************************************
 * process_command - Parse and execute user commands (COMPLETE)
 *
 * This function is provided complete for students. It handles:
 * - Command shortcuts (l for list, p for play, m for move)
 * - Full commands (list, play, move, help)
 * - Direct number input when in game (just type 1-9)
 *****************************************************************************/
void process_command(int socket, const char *input) {
    char arg1[MAX_INPUT];
    int position;

    /* Skip empty input */
    if (input[0] == '\0') {
        return;
    }

    /* Shortcut: If in game and input is just a number, treat as move */
    if (client_state == STATE_IN_GAME && sscanf(input, "%d", &position) == 1) {
        send_move(socket, position);
        return;
    }

    /* Help command */
    if (strcasecmp(input, "help") == 0 || strcasecmp(input, "h") == 0 || strcmp(input, "?") == 0) {
        printf("\n%s=== Commands ===%s\n", COLOR_CYAN, COLOR_RESET);
        printf("  %slist%s or %sl%s              - List all online players\n", COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        printf("  %splay <name>%s or %sp <name>%s - Start a game with someone\n", COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        if (client_state == STATE_IN_GAME) {
            printf("  %s1-9%s                   - Make a move (just type the number!)\n", COLOR_CYAN, COLOR_RESET);
        } else {
            printf("  %smove <1-9>%s or %sm <1-9>%s  - Make a move in your current game\n", COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        }
        printf("  %shelp%s, %sh%s, or %s?%s          - Show this help\n", COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
        printf("  %s^C%s                    - Exit\n\n", COLOR_CYAN, COLOR_RESET);
        return;
    }

    /* Shortcut: 'l' or 'list' for player list */
    if (strcasecmp(input, "l") == 0 || strcasecmp(input, "list") == 0) {
        send_list_request(socket);
        return;
    }

    /* Shortcut: 'play username' - check "play " first (longer match) */
    if (strncasecmp(input, "play ", 5) == 0 && sscanf(input + 5, "%100s", arg1) == 1) {
        send_game_start_request(socket, arg1);
        return;
    }

    /* Shortcut: 'p username' */
    if (input[0] == 'p' && input[1] == ' ' && sscanf(input + 2, "%100s", arg1) == 1) {
        send_game_start_request(socket, arg1);
        return;
    }

    /* Shortcut: 'move position' - check "move " first */
    if (strncasecmp(input, "move ", 5) == 0 && sscanf(input + 5, "%d", &position) == 1) {
        send_move(socket, position);
        return;
    }

    /* Shortcut: 'm position' */
    if (input[0] == 'm' && input[1] == ' ' && sscanf(input + 2, "%d", &position) == 1) {
        send_move(socket, position);
        return;
    }

    /* No valid command matched */
    printf("Invalid command. Try: %shelp%s, %slist%s, %splay <name>%s",
           COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET, COLOR_CYAN, COLOR_RESET);
    if (client_state == STATE_IN_GAME) {
        printf(", or just type a number (1-9)\n");
    } else {
        printf("\n");
    }
}

/*****************************************************************************
 * TODO: send_list_request - Send Flag 10
 *
 * This is the SIMPLEST packet format - just a flag byte!
 *
 * Flag 10 Packet Format:
 *   +------+
 *   | Flag |
 *   +------+
 *   | 1 B  |
 *   +------+
 *
 * Example:
 *   buffer[0] = 10      (FLAG_LIST_REQ)
 *   Total packet size: 1 byte
 *
 * Steps:
 *   1. Create a buffer (1 byte is enough)
 *   2. Set buffer[0] = FLAG_LIST_REQ
 *   3. Call sendPDU(socket, buffer, 1)
 *
 * This is the simplest packet type. Compare with send_initial_connection()
 * above to see the pattern, but this one has NO length or data fields.
 *****************************************************************************/
void send_list_request(int socket) {
    /* TODO: Build and send Flag 10 packet (just 1 byte!) */

    uint8_t buffer[1];
    buffer[0] = FLAG_LIST_REQ;
    sendPDU(socket, buffer, 1);

}

/*****************************************************************************
 * TODO: send_game_start_request - Send Flag 20
 *
 * This packet has the same structure as Flag 1 (see send_initial_connection).
 *
 * Flag 20 Packet Format:
 *   +------+-------------+------------------+
 *   | Flag | Opponent    | Opponent Name    |
 *   |      | Name Length | (variable)       |
 *   +------+-------------+------------------+
 *   | 1 B  | 1 B         | opponent_len B   |
 *   +------+-------------+------------------+
 *
 * Example for opponent "bob" (3 characters):
 *   buffer[0] = 20      (FLAG_GAME_START_REQ)
 *   buffer[1] = 3       (length of "bob")
 *   buffer[2] = 'b'
 *   buffer[3] = 'o'
 *   buffer[4] = 'b'
 *   Total packet size: 1 + 1 + 3 = 5 bytes
 *
 * Steps:
 *   1. Create a buffer (BUFFER_SIZE is safe)
 *   2. Get opponent name length: strlen(opponent)
 *   3. Set buffer[0] = FLAG_GAME_START_REQ
 *   4. Set buffer[1] = opponent_len
 *   5. Copy opponent name: memcpy(buffer + 2, opponent, opponent_len)
 *   6. Call sendPDU(socket, buffer, 2 + opponent_len)
 *
 * Compare with send_initial_connection() - it's the exact same pattern,
 * just a different flag and different data!
 *****************************************************************************/
void send_game_start_request(int socket, const char *opponent) {
    /* TODO: Build and send Flag 20 packet (flag + length + opponent name) */

    uint8_t buffer[BUFFER_SIZE];
    int opponent_len = strlen(opponent);
    buffer[0] = FLAG_GAME_START_REQ;
    buffer[1] = opponent_len;
    memcpy(buffer + 2, opponent, opponent_len);
    sendPDU(socket, buffer, 2 + opponent_len);

}

/*****************************************************************************
 * TODO: send_move - Send Flag 30
 *
 * This packet is different - instead of a length field, it has fixed data.
 *
 * Flag 30 Packet Format:
 *   +------+---------+----------+
 *   | Flag | Game ID | Position |
 *   +------+---------+----------+
 *   | 1 B  | 1 B     | 1 B      |
 *   +------+---------+----------+
 *
 * Example for game_id=5, position=7:
 *   buffer[0] = 30      (FLAG_MOVE)
 *   buffer[1] = 5       (game_id)
 *   buffer[2] = 7       (position 1-9)
 *   Total packet size: 3 bytes (always fixed!)
 *
 * Steps:
 *   1. Check if client_state == STATE_IN_GAME
 *      - If not, print "You are not in a game.\n" and return
 *   2. Create a buffer (3 bytes is enough)
 *   3. Set buffer[0] = FLAG_MOVE
 *   4. Set buffer[1] = current_game_id (cast to uint8_t)
 *   5. Set buffer[2] = position (cast to uint8_t)
 *   6. Call sendPDU(socket, buffer, 3)
 *
 * Note: This packet has NO length field because all data is fixed size!
 * Compare with Flag 1 and Flag 20 which have variable-length strings.
 *****************************************************************************/
void send_move(int socket, int position) {
    /* TODO: Build and send Flag 30 packet (flag + game_id + position) */

    if (client_state != STATE_IN_GAME) {
        printf("You are not in a game.\n");
        return;
    }

    uint8_t buffer[3];
    buffer[0] = FLAG_MOVE;
    buffer[1] = (uint8_t)current_game_id;
    buffer[2] = (uint8_t)position;
    sendPDU(sockert, buffer, 3);

}

/*****************************************************************************
 * handle_conn_accept - Process Flag 2 (COMPLETE)
 *
 * This is a COMPLETE EXAMPLE showing the SIMPLEST packet parsing.
 *
 * Flag 2 Packet Format:
 *   +------+
 *   | Flag |
 *   +------+
 *   | 1 B  |
 *   +------+
 *
 * This packet contains ONLY a flag - no other data to parse!
 * The packet was already validated in send_initial_connection(),
 * so we just need to display the welcome message.
 *****************************************************************************/
void handle_conn_accept(void) {
    /* No parsing needed - flag-only packet */
    printf("\n%s=== Welcome to Tic-Tac-Toe, %s! ===%s\n", COLOR_CYAN, my_username, COLOR_RESET);
    printf("Type %shelp%s or %s?%s for commands\n\n", COLOR_YELLOW, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
}

/*****************************************************************************
 * handle_conn_reject - Process Flag 3 (COMPLETE)
 *
 * This is a COMPLETE EXAMPLE showing how to parse: flag + length + string
 *
 * Flag 3 Packet Format:
 *   +------+-------------+------------------+
 *   | Flag | Username    | Username         |
 *   |      | Length      | (variable)       |
 *   +------+-------------+------------------+
 *   | 1 B  | 1 B         | username_len B   |
 *   +------+-------------+------------------+
 *
 * Example packet rejecting "alice":
 *   buffer[0] = 3       (FLAG_CONN_REJECT)
 *   buffer[1] = 5       (length)
 *   buffer[2] = 'a'
 *   buffer[3] = 'l'
 *   buffer[4] = 'i'
 *   buffer[5] = 'c'
 *   buffer[6] = 'e'
 *   len = 7 bytes total
 *
 * Parsing steps demonstrated:
 *   1. Validate minimum length (need at least flag + length = 2 bytes)
 *   2. Extract the length field
 *   3. Validate total length (need flag + length + data)
 *   4. Extract the string data
 *   5. Null-terminate the string for C
 *   6. Use the data
 *****************************************************************************/
void handle_conn_reject(uint8_t *buffer, int len) {
    uint8_t username_len;
    char username[101];

    /* Step 1: Validate minimum packet size */
    if (len < 2) {
        printf("Username already in use\n");
        return;
    }

    /* Step 2: Extract the length field */
    username_len = buffer[1];

    /* Step 3: Validate we have enough bytes for the full string */
    if (len >= 2 + username_len) {
        /* Step 4: Extract the string data */
        memcpy(username, buffer + 2, username_len);

        /* Step 5: Null-terminate for C string */
        username[username_len] = '\0';

        /* Step 6: Use the data */
        printf("Username already in use: %s\n", username);
    }
}

/*****************************************************************************
 * handle_list_response - Process Flag 11, 12, 13 sequence (COMPLETE)
 *
 * This is a COMPLETE EXAMPLE showing MULTI-PACKET parsing.
 * The list comes as a SEQUENCE of packets, not just one!
 *
 * Packet Sequence:
 *   1. Flag 11 - List count (tells us how many Flag 12's to expect)
 *   2. Flag 12 - Repeated 'count' times (one per user)
 *   3. Flag 13 - End of list marker
 *
 * Flag 11 Packet Format:
 *   +------+-------+
 *   | Flag | Count |
 *   +------+-------+
 *   | 1 B  | 4 B   |
 *   +------+-------+
 *   Count is a 4-byte integer in network byte order (use ntohl!)
 *
 * Flag 12 Packet Format (repeated 'count' times):
 *   +------+-------------+------------------+
 *   | Flag | Username    | Username         |
 *   |      | Length      | (variable)       |
 *   +------+-------------+------------------+
 *   | 1 B  | 1 B         | username_len B   |
 *   +------+-------------+------------------+
 *
 * Flag 13 Packet Format:
 *   +------+
 *   | Flag |
 *   +------+
 *   | 1 B  |
 *   +------+
 *
 * Example sequence for 2 users ("alice" and "bob"):
 *   Packet 1: [11][0][0][0][2]         - Flag 11, count=2
 *   Packet 2: [12][5][a][l][i][c][e]   - Flag 12, alice
 *   Packet 3: [12][3][b][o][b]         - Flag 12, bob
 *   Packet 4: [13]                     - Flag 13, done
 *****************************************************************************/
void handle_list_response(int socket, uint8_t *initial_buffer, int initial_len) {
    uint8_t buffer[BUFFER_SIZE];
    uint32_t count;
    int bytes_received;
    int i;

    /* Parse Flag 11: initial_buffer already contains it */
    if (initial_len < 5) {
        return;
    }

    /* Extract 4-byte count (network byte order!) */
    memcpy(&count, initial_buffer + 1, 4);
    count = ntohl(count);  /* Convert network to host byte order */

    printf("Players online (%d):\n", count);

    /* Receive and parse each Flag 12 packet */
    for (i = 0; i < (int)count; i++) {
        bytes_received = recvPDU(socket, buffer, BUFFER_SIZE);
        if (bytes_received < 2 || buffer[0] != FLAG_LIST_USER) {
            break;
        }

        /* Parse Flag 12: same format as Flag 3 (length + string) */
        uint8_t username_len = buffer[1];
        if (bytes_received >= 2 + username_len) {
            char username[101];
            memcpy(username, buffer + 2, username_len);
            username[username_len] = '\0';
            printf("  %s\n", username);
        }
    }

    /* Receive Flag 13 (end marker) - we don't need to process it */
    recvPDU(socket, buffer, BUFFER_SIZE);
}

/*****************************************************************************
 * TODO: handle_game_started - Process Flag 21
 *
 * This packet combines a variable-length string with fixed fields.
 *
 * Flag 21 Packet Format:
 *   +------+-------------+------------------+-----------+---------+
 *   | Flag | Opponent    | Opponent Name    | My Symbol | Game ID |
 *   |      | Name Length | (variable)       |           |         |
 *   +------+-------------+------------------+-----------+---------+
 *   | 1 B  | 1 B         | opponent_len B   | 1 B       | 1 B     |
 *   +------+-------------+------------------+-----------+---------+
 *
 * Symbol values: 0 = O (goes second), 1 = X (goes first)
 *
 * Example for opponent "bob", you are X, game_id=3:
 *   buffer[0] = 21      (FLAG_GAME_STARTED)
 *   buffer[1] = 3       (length of "bob")
 *   buffer[2] = 'b'
 *   buffer[3] = 'o'
 *   buffer[4] = 'b'
 *   buffer[5] = 1       (my_symbol = X)
 *   buffer[6] = 3       (game_id)
 *   Total length: 1 + 1 + 3 + 1 + 1 = 7 bytes
 *
 * Steps:
 *   1. Validate minimum length: need at least 4 bytes (flag+len+symbol+id)
 *   2. Extract opponent_len from buffer[1]
 *   3. Validate full length: need 4 + opponent_len bytes
 *   4. Extract opponent name from buffer[2...2+opponent_len]
 *   5. Null-terminate the opponent name
 *   6. Extract my_symbol from buffer[2 + opponent_len]
 *   7. Extract current_game_id from buffer[3 + opponent_len]
 *   8. Update globals: client_state = STATE_IN_GAME
 *   9. Call init_board() to start fresh
 *  10. Display appropriate message based on my_symbol:
 *      - If X (1): "You go first!" and show prompt
 *      - If O (0): "Waiting for opponent's move..."
 *
 * Compare with handle_conn_reject() for string parsing pattern.
 *****************************************************************************/
void handle_game_started(uint8_t *buffer, int len) {
    /* TODO: Parse Flag 21 and set up game state */

    if (len < 4) return;
    int opponent_len = buffer[1];
    if (len < 4 + opponent_len) return;
    char opponent_name[opponent_len + 1];
    memcpy(opponent_name, buffer + 2, opponent_len);
    opponent_name[opponent_len] = '\0';
    
    my_symbol = buffer[2 + opponent_len];
    current_game_id = buffer[3 + opponent_len];
    client_state = STATE_IN_GAME;
    init_board();
    if (my_symbol == 1) printf("You go first!\n");
    else printf("Waiting for opponent's move...\n");


    fprintf(stderr, "ERROR: handle_game_started() not yet implemented\n");
}

/*****************************************************************************
 * TODO: handle_game_start_error - Process Flag 22
 *
 * This packet has an error code BEFORE the length+string fields.
 *
 * Flag 22 Packet Format:
 *   +------+------------+-------------+------------------+
 *   | Flag | Error Code | Username    | Username         |
 *   |      |            | Length      | (variable)       |
 *   +------+------------+-------------+------------------+
 *   | 1 B  | 1 B        | 1 B         | username_len B   |
 *   +------+------------+-------------+------------------+
 *
 * Error codes:
 *   0 = Player does not exist
 *   1 = Player already in a game
 *   2 = You are already in a game
 *   3 = Cannot play against yourself
 *
 * Example rejecting play request for "alice" (player doesn't exist):
 *   buffer[0] = 22      (FLAG_GAME_START_ERR)
 *   buffer[1] = 0       (error code: does not exist)
 *   buffer[2] = 5       (length of "alice")
 *   buffer[3] = 'a'
 *   buffer[4] = 'l'
 *   buffer[5] = 'i'
 *   buffer[6] = 'c'
 *   buffer[7] = 'e'
 *   Total length: 1 + 1 + 1 + 5 = 8 bytes
 *
 * Steps:
 *   1. Validate minimum length: need at least 3 bytes (flag+error+len)
 *   2. Extract error_code from buffer[1]
 *   3. Extract username_len from buffer[2]
 *   4. Validate full length: need 3 + username_len bytes
 *   5. Extract username from buffer[3...3+username_len]
 *   6. Null-terminate the username
 *   7. Use a switch statement to display appropriate error message
 *
 * Compare with handle_conn_reject(), but note the error_code field comes
 * BEFORE the length field here!
 *****************************************************************************/
void handle_game_start_error(uint8_t *buffer, int len) {
    /* TODO: Parse Flag 22 and display error message */

    if (len < 3) return;
    int error_code = buffer[1];
    int username_len = buffer[2];
    if (len < 3 + username_len) return;
    char username[username_len + 1];
    memcpy(username, buffer + 3, username_len);
    username[username_len] = '\0';
    switch (error_code) {
        case 0:
            printf("Player does not exist: %s\n", username);
            break;
        case 1:
            printf("Player already in a game: %s\n", username);
            break;
        case 2:
            printf("You are already in a game: %s\n", username);
            break;
        case 3:
            printf("Cannot play against yourself: %s\n", username);
            break;
    }

}

/*****************************************************************************
 * TODO: handle_board_update - Process Flag 31
 *
 * This is the most complex packet - it contains multiple fixed fields
 * including an array of 9 bytes for the board state.
 *
 * Flag 31 Packet Format:
 *   +------+---------+----------+-----------+---------------+-----------+
 *   | Flag | Game ID | Position | Who Moved | Board State   | Next Turn |
 *   |      |         |          |           | (9 bytes)     |           |
 *   +------+---------+----------+-----------+---------------+-----------+
 *   | 1 B  | 1 B     | 1 B      | 1 B       | 9 B           | 1 B       |
 *   +------+---------+----------+-----------+---------------+-----------+
 *   Total: 14 bytes (always fixed!)
 *
 * Position: 1-9 (which square was just played)
 * Who moved: 0=O, 1=X (which symbol made the move)
 * Board state: 9 bytes, one per square (0=empty, 1=X, 2=O)
 *   Board positions: [0]=square 1, [1]=square 2, ... [8]=square 9
 * Next turn: 0=O, 1=X (whose turn is next)
 *
 * Example: X moves to position 5 (center), next turn is O
 *   buffer[0]  = 31     (FLAG_BOARD_UPDATE)
 *   buffer[1]  = 3      (game_id)
 *   buffer[2]  = 5      (position)
 *   buffer[3]  = 1      (who_moved: X)
 *   buffer[4]  = 0      (square 1: empty)
 *   buffer[5]  = 0      (square 2: empty)
 *   buffer[6]  = 0      (square 3: empty)
 *   buffer[7]  = 0      (square 4: empty)
 *   buffer[8]  = 1      (square 5: X)
 *   buffer[9]  = 0      (square 6: empty)
 *   buffer[10] = 0      (square 7: empty)
 *   buffer[11] = 0      (square 8: empty)
 *   buffer[12] = 0      (square 9: empty)
 *   buffer[13] = 0      (next_turn: O)
 *
 * Steps:
 *   1. Validate length: must be exactly 14 bytes
 *   2. Extract position from buffer[2]
 *   3. Extract who_moved from buffer[3]
 *   4. Copy board state: memcpy(board, buffer + 4, 9)
 *   5. Extract next_turn from buffer[13]
 *   6. Display the move:
 *      - If who_moved == my_symbol: "You placed X/O at position N"
 *      - Otherwise: "opponent placed X/O at position N"
 *   7. Call display_board() to show updated board
 *   8. Display whose turn is next:
 *      - If next_turn == my_symbol: prompt for move
 *      - Otherwise: "Waiting for opponent's move..."
 *
 * This packet has NO variable-length fields, just lots of fixed fields!
 *****************************************************************************/
void handle_board_update(uint8_t *buffer, int len) {
    /* TODO: Parse Flag 31 and update board display */

    if (len < 14) return;
    int position = buffer[2];
    int who_moved = buffer[3];
    memcpy(board, buffer + 4, 9);
    int next_turn = buffer[13];
    if (who_moved == my_symbol) printf("You placed X/0 at poisition %d\n", position);
    else printf("Opponent placed X/O at position %d\n", position);
    display_board();
    if (next_turn == my_symbol) printf("Your turn\n");
    else printf("Waiting for opponent's move..."\n);

}

/*****************************************************************************
 * TODO: handle_move_invalid - Process Flag 32
 *
 * Simple packet with just an error code.
 *
 * Flag 32 Packet Format:
 *   +------+------------+
 *   | Flag | Error Code |
 *   +------+------------+
 *   | 1 B  | 1 B        |
 *   +------+------------+
 *   Total: 2 bytes (always fixed!)
 *
 * Error codes:
 *   0 = Not your turn
 *   1 = Position already occupied
 *   2 = Invalid position (not 1-9)
 *   3 = Not in a game
 *
 * Example rejecting move (not your turn):
 *   buffer[0] = 32      (FLAG_MOVE_INVALID)
 *   buffer[1] = 0       (error code: not your turn)
 *
 * Steps:
 *   1. Validate length: need at least 2 bytes
 *   2. Extract error_code from buffer[1]
 *   3. Use a switch statement to display appropriate error:
 *      case 0: "It's not your turn!\n"
 *      case 1: "Position already occupied.\n"
 *      case 2: "Invalid position. Choose 1-9.\n"
 *      case 3: "You are not in a game.\n"
 *      default: "Invalid move.\n"
 *
 * This is similar to Flag 2 (simple) but with an error code field.
 *****************************************************************************/
void handle_move_invalid(uint8_t *buffer, int len) {
    /* TODO: Parse Flag 32 and display error message */

    if (len < 2) return;
    int error_code = buffer[1];
    switch (error_code) {
        case 0:
            printf("It's not your turn!\n");
            break;
        case 1:
            printf("Position already occupied.\n");
            break;
        case 2:
            printf("Invalid position. Choose 1-9.\n");
            break;
        case 3:
            printf("You are not in a game.\n");
            break;
        default:
            printf("Invalid move.\n");
            break;
    }
}

/*****************************************************************************
 * TODO: handle_game_over - Process Flag 33
 *
 * Game over packet with result code and final board state.
 *
 * Flag 33 Packet Format:
 *   +------+---------+--------+--------------------+
 *   | Flag | Game ID | Result | Final Board State  |
 *   |      |         |        | (9 bytes)          |
 *   +------+---------+--------+--------------------+
 *   | 1 B  | 1 B     | 1 B    | 9 B                |
 *   +------+---------+--------+--------------------+
 *   Total: 12 bytes (always fixed!)
 *
 * Result codes:
 *   0 = Draw
 *   1 = X won
 *   2 = O won
 *   3 = X disconnected before game started
 *   4 = O disconnected before game started
 *   5 = X disconnected during game
 *   6 = O disconnected during game
 *
 * Example: X won the game
 *   buffer[0]  = 33     (FLAG_GAME_OVER)
 *   buffer[1]  = 3      (game_id)
 *   buffer[2]  = 1      (result: X won)
 *   buffer[3]  = 1      (square 1: X)
 *   buffer[4]  = 1      (square 2: X)
 *   buffer[5]  = 1      (square 3: X)
 *   buffer[6]  = 2      (square 4: O)
 *   buffer[7]  = 2      (square 5: O)
 *   buffer[8]  = 0      (square 6: empty)
 *   buffer[9]  = 0      (square 7: empty)
 *   buffer[10] = 0      (square 8: empty)
 *   buffer[11] = 0      (square 9: empty)
 *
 * Steps:
 *   1. Validate length: must be exactly 12 bytes
 *   2. Extract result from buffer[2]
 *   3. Copy board state: memcpy(board, buffer + 3, 9)
 *   4. Display result message using switch statement:
 *      case 0: "Draw game!"
 *      case 1: "You won!" if my_symbol==1, else "You lost!"
 *      case 2: "You won!" if my_symbol==0, else "You lost!"
 *      case 3-6: "Opponent disconnected" (check if you won by forfeit)
 *   5. Display final board: call display_board()
 *   6. Reset game state:
 *      - client_state = STATE_AVAILABLE
 *      - current_game_id = -1
 *      - my_symbol = -1
 *
 * Similar structure to Flag 31, with fixed fields including board array.
 *****************************************************************************/
void handle_game_over(uint8_t *buffer, int len) {
    /* TODO: Parse Flag 33, display result, and reset game state */

    if (len != 12) return;
    int result = buffer[2];
    memcpy(board, buffer + 3, 9);
    switch (result) {
        case 0:
            printf("Draw game!\n");
            break;
        case 1:
            printf(my_symbol == 1 ? "You won!\n" : "You lost!\n");
            break;
        case 2:
            printf(my_symbol == 0 ? "You won!\n" : "You lost!\n");
            break;
        case 3:
            printf("Opponent disconnected before game started.\n");
            break;
        case 4:
            printf("Opponent disconnected before game started.\n");
            break;
        case 5:
            printf("Opponent disconnected during game.\n");
            break;
        case 6:
            printf("Opponent disconnected during game.\n");
            break;
    }

    display_board();
    client_state = STATE_AVAILABLE;
    current_game_id = -1;
    my_symbol = -1;

}

/*****************************************************************************
 * display_board - Display the tic-tac-toe board with colors (COMPLETE)
 *
 * Board layout:
 *   1 | 2 | 3
 *  ---+---+---
 *   4 | 5 | 6
 *  ---+---+---
 *   7 | 8 | 9
 *
 * Colors:
 *   - Empty positions (0): Cyan numbers
 *   - X (1): Red
 *   - O (2): Blue
 *   - Grid lines: Yellow
 *****************************************************************************/
void display_board(void) {
    int i, j;

    for (i = 0; i < 3; i++) {
        printf(" ");
        for (j = 0; j < 3; j++) {
            int pos = i * 3 + j;
            if (board[pos] == 0) {
                /* Empty positions in cyan */
                printf("%s%d%s", COLOR_CYAN, pos + 1, COLOR_RESET);
            } else if (board[pos] == 1) {
                /* X in red */
                printf("%sX%s", COLOR_RED, COLOR_RESET);
            } else {
                /* O in blue */
                printf("%sO%s", COLOR_BLUE, COLOR_RESET);
            }

            if (j < 2) {
                printf(" %s|%s ", COLOR_YELLOW, COLOR_RESET);
            }
        }
        printf("\n");

        if (i < 2) {
            printf("%s---+---+---%s\n", COLOR_YELLOW, COLOR_RESET);
        }
    }
}

/*****************************************************************************
 * init_board - Initialize empty board (COMPLETE)
 *****************************************************************************/
void init_board(void) {
    memset(board, 0, 9);
}
