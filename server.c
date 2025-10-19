/*****************************************************************************
 * server.c - Tic-Tac-Toe server implementation (SKELETON VERSION)
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#include "pdu.h"
#include "users.h"
#include "game.h"

/* Packet flags - these define the protocol message types */
#define FLAG_INITIAL_CONN      1   /* Client sends username to connect */
#define FLAG_CONN_ACCEPT       2   /* Server accepts connection */
#define FLAG_CONN_REJECT       3   /* Server rejects connection */
#define FLAG_ERROR             7   /* Generic error message */
#define FLAG_LIST_REQ          10  /* Client requests player list */
#define FLAG_LIST_COUNT        11  /* Server sends player count */
#define FLAG_LIST_USER         12  /* Server sends one player name */
#define FLAG_LIST_DONE         13  /* Server signals end of list */
#define FLAG_GAME_START_REQ    20  /* Client requests to start game */
#define FLAG_GAME_STARTED      21  /* Server confirms game started */
#define FLAG_GAME_START_ERR    22  /* Server rejects game start */
#define FLAG_MOVE              30  /* Client sends move */
#define FLAG_BOARD_UPDATE      31  /* Server sends board state */
#define FLAG_MOVE_INVALID      32  /* Server rejects move */
#define FLAG_GAME_OVER         33  /* Server signals game end */

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

/* Global flag for graceful shutdown */
static volatile int keep_running = 1;

/* Function prototypes */
int setup_server(uint16_t port);
void run_server(int server_socket);
void handle_new_connection(int server_socket, struct pollfd *pfds, int *num_fds);
void handle_client_data(int index, struct pollfd *pfds, int *num_fds);
void handle_initial_connection(int socket, uint8_t *buffer, int len);
void handle_list_request(int socket);
void handle_game_start_request(int socket, uint8_t *buffer, int len);
void handle_move(int socket, uint8_t *buffer, int len);
void send_game_started(int x_socket, int o_socket, int game_id);
void send_board_update(int game_id, int position, int who_moved);
void send_game_over(int game_id, int result);
void handle_disconnect(int socket, struct pollfd *pfds, int *num_fds, int index);
void signal_handler(int signum);
int validate_username(const char *username);

/*****************************************************************************
 * main - Server entry point
 *
 * This function demonstrates proper initialization order and cleanup.
 * It's kept minimal to show the overall server structure.
 *****************************************************************************/
int main(int argc, char *argv[]) {
    uint16_t port = 0;
    int server_socket;

    /* Parse command line: optional port number */
    if (argc == 2) {
        port = (uint16_t)atoi(argv[1]);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    /* Setup signal handlers for clean shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Initialize data structure modules */
    users_init();
    game_init();

    /* Create and configure server socket */
    server_socket = setup_server(port);

    /* Run main server loop until signal received */
    run_server(server_socket);

    /* Clean shutdown: close socket and free resources */
    close(server_socket);
    users_cleanup();
    game_cleanup();

    return 0;
}

/*****************************************************************************
 * signal_handler - Handle SIGINT and SIGTERM for graceful shutdown
 *
 * When the user presses Ctrl-C (SIGINT) or the process receives SIGTERM,
 * this handler sets the keep_running flag to 0, which causes the main
 * server loop to exit gracefully.
 *****************************************************************************/
void signal_handler(int signum) {
    printf("\nReceived signal %d, shutting down gracefully...\n", signum);
    keep_running = 0;
}

/*****************************************************************************
 * TODO: setup_server - Create, bind, and configure the server socket
 *
 * This function must:
 * 1. Create a TCP socket using socket(AF_INET, SOCK_STREAM, 0)
 *    - Check for errors and exit(1) if socket creation fails
 *
 * 2. Set SO_REUSEADDR socket option to allow quick restarts
 *    - Use setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, ...)
 *    - This prevents "Address already in use" errors
 *
 * 3. Bind the socket to INADDR_ANY and the specified port
 *    - Create a struct sockaddr_in with:
 *      - sin_family = AF_INET
 *      - sin_addr.s_addr = INADDR_ANY (accept connections on any interface)
 *      - sin_port = htons(port) (convert port to network byte order)
 *    - Call bind() with this structure
 *    - Check for errors and exit(1) if bind fails
 *
 * 4. If port was 0 (auto-assign), get the actual port number
 *    - Use getsockname() to retrieve the assigned port
 *    - Print the port with: printf("Server is using port %d\n", ntohs(addr.sin_port));
 *
 * 5. Put the socket in listening mode
 *    - Call listen(socket, 10) to allow up to 10 pending connections
 *    - Check for errors and exit(1) if listen fails
 *
 * 6. Return the server socket descriptor
 *
 * Parameters:
 *   port - The port to bind to (0 = let OS choose)
 *
 * Returns:
 *   The server socket descriptor
 *****************************************************************************/
int setup_server(uint16_t port) {
    /* TODO: Implement this function */
    /* See the function header above for detailed implementation steps */

    int sockfd;
    struct sockaddr_in addr;
    int opt = 1;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (port == 0) {
        socklen_t len = sizeof(addr);
        if (getsockname(sockfd, (struct sockaddr *)&addr, &len) == -1) {
            perror("getsockname");
            close(sockfd);
            exit(1);
        }
        printf("Server is using port %d\n", ntohs(addr.sin_port));
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    return sockfd;
}

/*****************************************************************************
 * TODO: run_server - Main server loop using poll()
 *
 * This function implements the core server logic using poll() for I/O multiplexing.
 *
 * Implementation steps:
 * 1. Create an array of struct pollfd[MAX_CLIENTS + 1]
 *    - Index 0 is always the server socket
 *    - Indices 1..num_fds-1 are client sockets
 *    - Initialize with memset() to zero
 *
 * 2. Setup the server socket in pfds[0]
 *    - pfds[0].fd = server_socket
 *    - pfds[0].events = POLLIN (wait for incoming data/connections)
 *    - Initialize num_fds = 1
 *
 * 3. Main loop: while (keep_running)
 *    a. Call poll(pfds, num_fds, -1)
 *       - This blocks until activity on any socket
 *       - -1 timeout means wait indefinitely
 *       - Returns number of sockets with activity
 *       - If poll() returns < 0, print error and continue
 *
 *    b. Check server socket (pfds[0]) for new connections
 *       - If pfds[0].revents & POLLIN:
 *         - Call handle_new_connection(server_socket, pfds, &num_fds)
 *
 *    c. Check all client sockets for data
 *       - Loop: for (i = 1; i < num_fds; i++)
 *       - If pfds[i].revents & POLLIN:
 *         - Call handle_client_data(i, pfds, &num_fds)
 *       - Note: handle_client_data might remove a socket from the array,
 *               which could affect num_fds but not the current loop iteration
 *
 * Parameters:
 *   server_socket - The listening socket descriptor
 *****************************************************************************/
void run_server(int server_socket) {
    /* TODO: Implement this function */
    /* See the function header above for detailed implementation steps */

    struct pollfd pfds[MAX_CLIENTS + 1];

    memset(pfds, 0, sizeof(pfds));

    pfds[0].fd = server_socket;
    pfds[0].events = POLLIN;
    int num_fds = 1;

    while (keep_running) {
        int poll_count = poll(pfds, num_fds, -1);
        if (poll_count < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (pfds[0].revents && POLLIN) handle_new_connection(server_socket, pfds, &num_fds);

        for (int i = 1; i < num_fds; i++) {
            if (pfds[i].revents && POLLIN) handle_client_data(i, pfds, &num_fds);
        }
    }
}

/*****************************************************************************
 * TODO: handle_new_connection - Accept a new client connection
 *
 * This function is called when poll() detects activity on the server socket,
 * indicating a client is trying to connect.
 *
 * Implementation steps:
 * 1. Prepare for accept()
 *    - Declare: struct sockaddr_in client_addr;
 *    - Declare: socklen_t addr_len = sizeof(client_addr);
 *
 * 2. Accept the connection
 *    - Call accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)
 *    - This creates a new socket for communicating with this client
 *    - If accept() returns < 0, print error with perror("accept") and return
 *
 * 3. Add the new client socket to the poll array
 *    - Check if there's room: if (*num_fds < MAX_CLIENTS + 1)
 *    - If yes:
 *      - pfds[*num_fds].fd = client_socket
 *      - pfds[*num_fds].events = POLLIN
 *      - (*num_fds)++
 *    - If no room:
 *      - Print "Too many clients\n" to stderr
 *      - close(client_socket) to reject the connection
 *
 * Note: At this point, we have accepted the TCP connection but the client
 * hasn't sent their username yet. That will be handled when they send
 * their first packet (Flag 1).
 *
 * Parameters:
 *   server_socket - The listening socket
 *   pfds          - Array of poll file descriptors
 *   num_fds       - Pointer to number of active file descriptors
 *****************************************************************************/
void handle_new_connection(int server_socket, struct pollfd *pfds, int *num_fds) {
    /* TODO: Implement this function */
    /* See the function header above for detailed implementation steps */

    fprintf(stderr, "ERROR: handle_new_connection() not implemented\n");
}

/*****************************************************************************
 * TODO: handle_client_data - Receive and process a packet from a client
 *
 * This function is called when poll() detects data available on a client socket.
 * It receives one complete packet and dispatches it to the appropriate handler.
 *
 * Implementation steps:
 * 1. Get the socket descriptor from pfds[index].fd
 *
 * 2. Receive a packet using recvPDU()
 *    - Declare: uint8_t buffer[BUFFER_SIZE];
 *    - Call: int bytes_received = recvPDU(socket, buffer, BUFFER_SIZE);
 *
 * 3. Handle receive results:
 *    - If bytes_received == 0:
 *      - Client disconnected gracefully
 *      - Call handle_disconnect(socket, pfds, num_fds, index)
 *      - return
 *
 *    - If bytes_received < 0:
 *      - Error occurred (timeout, connection reset, etc.)
 *      - Call handle_disconnect(socket, pfds, num_fds, index)
 *      - return
 *
 * 4. Parse and dispatch the packet
 *    - Extract flag: uint8_t flag = buffer[0];
 *    - Use a switch statement on flag:
 *
 *      case FLAG_INITIAL_CONN (1):
 *          handle_initial_connection(socket, buffer, bytes_received);
 *          break;
 *
 *      case FLAG_LIST_REQ (10):
 *          handle_list_request(socket);
 *          break;
 *
 *      case FLAG_GAME_START_REQ (20):
 *          handle_game_start_request(socket, buffer, bytes_received);
 *          break;
 *
 *      case FLAG_MOVE (30):
 *          handle_move(socket, buffer, bytes_received);
 *          break;
 *
 *      default:
 *          fprintf(stderr, "Unknown flag: %d\n", flag);
 *          break;
 *
 * Parameters:
 *   index   - Index in pfds array for this client
 *   pfds    - Array of poll file descriptors
 *   num_fds - Pointer to number of active file descriptors
 *****************************************************************************/
void handle_client_data(int index, struct pollfd *pfds, int *num_fds) {
    /* TODO: Implement this function */
    /* See the function header above for detailed implementation steps */

    fprintf(stderr, "ERROR: handle_client_data() not implemented\n");
}

/*****************************************************************************
 * handle_initial_connection - Process Flag 1 (initial connection packet)
 *
 * *** THIS IS THE MASTER EXAMPLE - STUDY IT CAREFULLY ***
 *
 * This function demonstrates EVERYTHING you need to know about protocol handling:
 * - How to parse incoming packets (extract length prefix, validate, extract data)
 * - How to use the users module (users_exists, users_add)
 * - How to build response packets with proper format
 * - Error handling and validation
 * - Using sendPDU() to send responses
 *
 * Use this as your reference when implementing the other protocol handlers!
 *
 * INCOMING PACKET FORMAT (Flag 1):
 * +------+--------+----------+
 * | Flag | Length | Username |
 * +------+--------+----------+
 * | 1 B  | 1 B    | N bytes  |
 * +------+--------+----------+
 *
 * Byte layout:
 *   [0]    = FLAG_INITIAL_CONN (1)
 *   [1]    = username_length (1 byte, value N)
 *   [2..N+1] = username (N bytes, NOT null-terminated in packet)
 *
 * RESPONSE PACKET FORMATS:
 *
 * Success (Flag 2):
 * +------+
 * | Flag |
 * +------+
 * | 1 B  |
 * +------+
 *   [0] = FLAG_CONN_ACCEPT (2)
 *
 * Rejection (Flag 3):
 * +------+--------+----------+
 * | Flag | Length | Username |
 * +------+--------+----------+
 * | 1 B  | 1 B    | N bytes  |
 * +------+--------+----------+
 *   [0]    = FLAG_CONN_REJECT (3)
 *   [1]    = username_length
 *   [2..N+1] = username that was rejected
 *
 * EXAMPLE WITH REAL BYTES:
 * Client sends username "Alice":
 *   [01][05][41 6C 69 63 65]
 *    ^   ^   ^-- "Alice" in ASCII
 *    |   +------ Length: 5
 *    +---------- Flag 1
 *
 * Server accepts:
 *   [02]
 *    ^-- Flag 2
 *
 * Or server rejects:
 *   [03][05][41 6C 69 63 65]
 *    ^   ^   ^-- "Alice" in ASCII
 *    |   +------ Length: 5
 *    +---------- Flag 3
 *****************************************************************************/
void handle_initial_connection(int socket, uint8_t *buffer, int len) {
    uint8_t response[BUFFER_SIZE];
    uint8_t username_len;
    char username[101];  /* Max 100 chars + null terminator */
    int result;

    /* STEP 1: Validate minimum packet size */
    if (len < 2) {
        return;  /* Malformed packet - need at least flag + length */
    }

    /* STEP 2: Parse username length from packet */
    username_len = buffer[1];

    /* STEP 3: Validate packet is large enough for the claimed username */
    if (len < 2 + username_len) {
        return;  /* Packet too short for claimed username length */
    }

    /* STEP 4: Extract username and null-terminate it for C string functions */
    memcpy(username, buffer + 2, username_len);
    username[username_len] = '\0';

    /* STEP 5: Validate username format (alphanumeric, starts with letter) */
    if (!validate_username(username)) {
        /* Build and send Flag 3 (connection rejected) */
        response[0] = FLAG_CONN_REJECT;
        response[1] = username_len;
        memcpy(response + 2, username, username_len);
        sendPDU(socket, response, 2 + username_len);
        printf("Username %s rejected (invalid format)\n", username);
        return;
    }

    /* STEP 6: Try to add user to the users table */
    result = users_add(username, socket);

    if (result == 0) {
        /* Success - send Flag 2 (connection accepted) */
        response[0] = FLAG_CONN_ACCEPT;
        sendPDU(socket, response, 1);
        printf("Player %s connected\n", username);
    } else {
        /* Username already exists - send Flag 3 (connection rejected) */
        response[0] = FLAG_CONN_REJECT;
        response[1] = username_len;
        memcpy(response + 2, username, username_len);
        sendPDU(socket, response, 2 + username_len);
        printf("Username %s rejected (already exists)\n", username);
    }
}

/*****************************************************************************
 * validate_username - Validate username according to protocol rules
 *
 * This helper function checks if a username meets the protocol requirements.
 *
 * Username rules:
 * - Length: 1-100 characters
 * - First character: must be a letter (a-z, A-Z)
 * - Remaining characters: letters, numbers, or underscore
 *
 * Returns:
 *   1 if valid
 *   0 if invalid
 *****************************************************************************/
int validate_username(const char *username) {
    int len;
    int i;

    if (!username) {
        return 0;  /* Invalid: NULL username */
    }

    len = strlen(username);

    /* Check length: 1-100 characters */
    if (len < 1 || len > 100) {
        return 0;  /* Invalid: wrong length */
    }

    /* Check first character: must be a letter */
    if (!isalpha(username[0])) {
        return 0;  /* Invalid: doesn't start with letter */
    }

    /* Check remaining characters: letters, numbers, underscores only */
    for (i = 1; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;  /* Invalid: contains invalid character */
        }
    }

    return 1;  /* Valid username */
}

/*****************************************************************************
 * TODO: handle_list_request - Process Flag 10 (player list request)
 *
 * This function must send a list of all connected players to the requesting client.
 * Study handle_initial_connection() to see how to build and send packets!
 *
 * INCOMING PACKET FORMAT (Flag 10):
 * +------+
 * | Flag |
 * +------+
 * | 1 B  |
 * +------+
 *   [0] = FLAG_LIST_REQ (10)
 *
 * OUTGOING PACKET SEQUENCE:
 *
 * 1. Flag 11 - Player count:
 * +------+-------+
 * | Flag | Count |
 * +------+-------+
 * | 1 B  | 4 B   |
 * +------+-------+
 *   [0]   = FLAG_LIST_COUNT (11)
 *   [1-4] = count (uint32_t in NETWORK byte order - use htonl!)
 *
 * 2. Flag 12 - One per player (send count times):
 * +------+--------+----------+
 * | Flag | Length | Username |
 * +------+--------+----------+
 * | 1 B  | 1 B    | N bytes  |
 * +------+--------+----------+
 *   [0]    = FLAG_LIST_USER (12)
 *   [1]    = username_length
 *   [2..N+1] = username
 *
 * 3. Flag 13 - End of list:
 * +------+
 * | Flag |
 * +------+
 * | 1 B  |
 * +------+
 *   [0] = FLAG_LIST_DONE (13)
 *
 * EXAMPLE WITH 3 PLAYERS (Alice, Bob, Carol):
 *
 * Packet 1 (Flag 11 - count):
 *   [0B][00 00 00 03]
 *    ^   ^---------- Count: 3 (in network order)
 *    +-------------- Flag 11
 *
 * Packet 2 (Flag 12 - Alice):
 *   [0C][05][41 6C 69 63 65]
 *    ^   ^   ^-- "Alice"
 *    |   +------ Length: 5
 *    +---------- Flag 12
 *
 * Packet 3 (Flag 12 - Bob):
 *   [0C][03][42 6F 62]
 *    ^   ^   ^-- "Bob"
 *    |   +------ Length: 3
 *    +---------- Flag 12
 *
 * Packet 4 (Flag 12 - Carol):
 *   [0C][05][43 61 72 6F 6C]
 *    ^   ^   ^-- "Carol"
 *    |   +------ Length: 5
 *    +---------- Flag 12
 *
 * Packet 5 (Flag 13 - done):
 *   [0D]
 *    ^-- Flag 13
 *
 * IMPLEMENTATION STEPS:
 * 1. Allocate space for username strings
 *    - Declare: char *usernames[MAX_CLIENTS];
 *    - Use malloc(101) for each pointer (100 + null terminator)
 *    - Remember to free() all allocated memory before returning!
 *
 * 2. Get all usernames from the users module
 *    - Call: int count = users_get_all(usernames, MAX_CLIENTS);
 *    - This fills the usernames array and returns the count
 *
 * 3. Send Flag 11 (player count)
 *    - Convert count to network byte order: uint32_t net_count = htonl(count);
 *    - buffer[0] = FLAG_LIST_COUNT
 *    - memcpy(buffer + 1, &net_count, 4)
 *    - sendPDU(socket, buffer, 5)
 *
 * 4. Send Flag 12 for each player
 *    - Loop: for (i = 0; i < count; i++)
 *    - For each player:
 *      - uint8_t len = strlen(usernames[i])
 *      - buffer[0] = FLAG_LIST_USER
 *      - buffer[1] = len
 *      - memcpy(buffer + 2, usernames[i], len)
 *      - sendPDU(socket, buffer, 2 + len)
 *
 * 5. Send Flag 13 (end of list)
 *    - buffer[0] = FLAG_LIST_DONE
 *    - sendPDU(socket, buffer, 1)
 *
 * 6. Free all allocated memory
 *    - Loop through usernames array and free() each pointer
 *
 * USERS MODULE FUNCTIONS YOU'LL NEED:
 *   int users_get_all(char **usernames, int max_users);
 *   - Fills the usernames array with all connected player names
 *   - Returns the number of players
 *
 * Parameters:
 *   socket - The requesting client's socket
 *****************************************************************************/
void handle_list_request(int socket) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() */
    /* See the detailed packet formats and implementation steps above */

    fprintf(stderr, "ERROR: handle_list_request() not implemented\n");
}

/*****************************************************************************
 * TODO: handle_game_start_request - Process Flag 20 (game start request)
 *
 * This function processes a request from one player to start a game with another.
 * It performs extensive validation and coordinates between two players.
 *
 * INCOMING PACKET FORMAT (Flag 20):
 * +------+--------+-------------------+
 * | Flag | Length | Opponent Username |
 * +------+--------+-------------------+
 * | 1 B  | 1 B    | N bytes           |
 * +------+--------+-------------------+
 *   [0]    = FLAG_GAME_START_REQ (20)
 *   [1]    = opponent_username_length
 *   [2..N+1] = opponent_username
 *
 * EXAMPLE - Alice challenges Bob:
 *   [14][03][42 6F 62]
 *    ^   ^   ^-- "Bob"
 *    |   +------ Length: 3
 *    +---------- Flag 20 (0x14 in hex)
 *
 * OUTGOING PACKETS:
 *
 * Success (Flag 21) - sent to BOTH players:
 * +------+--------+-------------------+--------+---------+
 * | Flag | Length | Opponent Username | Symbol | Game ID |
 * +------+--------+-------------------+--------+---------+
 * | 1 B  | 1 B    | N bytes           | 1 B    | 1 B     |
 * +------+--------+-------------------+--------+---------+
 *   [0]    = FLAG_GAME_STARTED (21)
 *   [1]    = opponent_username_length
 *   [2..N+1] = opponent_username (the OTHER player's name)
 *   [N+2]  = your_symbol (SYMBOL_X=1 or SYMBOL_O=0)
 *   [N+3]  = game_id
 *
 * Note: The challenger gets SYMBOL_X, the challenged gets SYMBOL_O
 * Note: Each player receives the OTHER player's name as the opponent
 *
 * Error (Flag 22):
 * +------+------------+--------+-------------------+
 * | Flag | Error Code | Length | Opponent Username |
 * +------+------------+--------+-------------------+
 * | 1 B  | 1 B        | 1 B    | N bytes           |
 * +------+------------+--------+-------------------+
 *   [0]   = FLAG_GAME_START_ERR (22)
 *   [1]   = error_code (see below)
 *   [2]   = opponent_username_length
 *   [3..N+2] = opponent_username
 *
 * ERROR CODES:
 *   0 = Player doesn't exist
 *   1 = Player is already in a game
 *   2 = You are already in a game
 *   3 = Cannot play against yourself
 *
 * EXAMPLE - Alice successfully challenges Bob, game_id=5:
 * To Alice (challenger):
 *   [15][03][42 6F 62][01][05]
 *    ^   ^   ^--Bob   ^   ^-- Game ID: 5
 *    |   |            +------ Symbol: X (1)
 *    |   +------------------- Length: 3
 *    +----------------------- Flag 21
 *
 * To Bob (challenged):
 *   [15][05][41 6C 69 63 65][00][05]
 *    ^   ^   ^--Alice  ^   ^-- Game ID: 5
 *    |   |            +------ Symbol: O (0)
 *    |   +------------------- Length: 5
 *    +----------------------- Flag 21
 *
 * EXAMPLE - Error: Bob doesn't exist:
 *   [16][00][03][42 6F 62]
 *    ^   ^   ^   ^-- "Bob"
 *    |   |   +------ Length: 3
 *    |   +---------- Error code: 0 (doesn't exist)
 *    +-------------- Flag 22
 *
 * IMPLEMENTATION STEPS:
 * 1. Parse the opponent username from the packet
 *    - Extract username_len from buffer[1]
 *    - Validate packet size (len >= 2 + username_len)
 *    - Copy username and null-terminate it
 *
 * 2. Get the requester's username
 *    - Declare: char requester_username[101];
 *    - Call: users_get_username(socket, requester_username)
 *    - If it returns < 0, this socket isn't registered (shouldn't happen)
 *
 * 3. Validate the request (check in this order):
 *    a. Check if trying to play self:
 *       - if (strcmp(opponent_username, requester_username) == 0)
 *       - Send Flag 22 with error code 3
 *
 *    b. Check if opponent exists:
 *       - if (!users_exists(opponent_username))
 *       - Send Flag 22 with error code 0
 *
 *    c. Check if requester is already in a game:
 *       - if (users_get_state(requester_username) == USER_IN_GAME)
 *       - Send Flag 22 with error code 2
 *
 *    d. Check if opponent is available:
 *       - if (users_get_state(opponent_username) == USER_IN_GAME)
 *       - Send Flag 22 with error code 1
 *
 * 4. If all validations pass, create the game
 *    a. Get opponent socket:
 *       - int opponent_socket = users_get_socket(opponent_username);
 *
 *    b. Create the game (requester is X, opponent is O):
 *       - int game_id = game_create(socket, opponent_socket);
 *       - Check if game_id < 0 (creation failed)
 *
 *    c. Update both players' states:
 *       - users_set_state(requester_username, USER_IN_GAME)
 *       - users_set_state(opponent_username, USER_IN_GAME)
 *
 *    d. Send game started notification to both:
 *       - Call: send_game_started(socket, opponent_socket, game_id)
 *       - (You'll implement send_game_started separately)
 *
 * USERS MODULE FUNCTIONS YOU'LL NEED:
 *   int users_get_username(int socket, char *username);
 *   int users_exists(const char *username);
 *   UserState users_get_state(const char *username);
 *   int users_get_socket(const char *username);
 *   void users_set_state(const char *username, UserState state);
 *
 * GAME MODULE FUNCTIONS YOU'LL NEED:
 *   int game_create(int x_socket, int o_socket);
 *
 * Parameters:
 *   socket - The requesting client's socket
 *   buffer - The received packet
 *   len    - Length of the received packet
 *****************************************************************************/
void handle_game_start_request(int socket, uint8_t *buffer, int len) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() for parsing */
    /* See the detailed packet formats and implementation steps above */

    fprintf(stderr, "ERROR: handle_game_start_request() not implemented\n");
}

/*****************************************************************************
 * TODO: send_game_started - Send Flag 21 to both players
 *
 * This function notifies both players that the game has started.
 * Each player receives a customized packet with:
 * - The opponent's username
 * - Their own symbol (X or O)
 * - The game ID
 *
 * PACKET FORMAT (Flag 21):
 * +------+--------+-------------------+--------+---------+
 * | Flag | Length | Opponent Username | Symbol | Game ID |
 * +------+--------+-------------------+--------+---------+
 * | 1 B  | 1 B    | N bytes           | 1 B    | 1 B     |
 * +------+--------+-------------------+--------+---------+
 *   [0]    = FLAG_GAME_STARTED (21)
 *   [1]    = opponent_username_length
 *   [2..N+1] = opponent_username
 *   [N+2]  = your_symbol (SYMBOL_X=1 or SYMBOL_O=0)
 *   [N+3]  = game_id
 *
 * EXAMPLE - Game 5 between Alice (X) and Bob (O):
 * To Alice:
 *   [15][03][42 6F 62][01][05]
 *    ^   ^   ^--Bob   ^   ^-- Game ID: 5
 *    |   |            +------ Symbol: X (1)
 *    |   +------------------- Length: 3
 *    +----------------------- Flag 21
 *
 * To Bob:
 *   [15][05][41 6C 69 63 65][00][05]
 *    ^   ^   ^--Alice  ^   ^-- Game ID: 5
 *    |   |            +------ Symbol: O (0)
 *    |   +------------------- Length: 5
 *    +----------------------- Flag 21
 *
 * IMPLEMENTATION STEPS:
 * 1. Get both players' usernames
 *    - Declare: char x_username[101], o_username[101];
 *    - Call: users_get_username(x_socket, x_username)
 *    - Call: users_get_username(o_socket, o_username)
 *
 * 2. Send to X player (challenger)
 *    - buffer[0] = FLAG_GAME_STARTED
 *    - buffer[1] = strlen(o_username)
 *    - memcpy(buffer + 2, o_username, strlen(o_username))
 *    - buffer[2 + strlen(o_username)] = SYMBOL_X
 *    - buffer[3 + strlen(o_username)] = (uint8_t)game_id
 *    - sendPDU(x_socket, buffer, 4 + strlen(o_username))
 *
 * 3. Send to O player (challenged)
 *    - buffer[0] = FLAG_GAME_STARTED
 *    - buffer[1] = strlen(x_username)
 *    - memcpy(buffer + 2, x_username, strlen(x_username))
 *    - buffer[2 + strlen(x_username)] = SYMBOL_O
 *    - buffer[3 + strlen(x_username)] = (uint8_t)game_id
 *    - sendPDU(o_socket, buffer, 4 + strlen(x_username))
 *
 * USERS MODULE FUNCTIONS YOU'LL NEED:
 *   int users_get_username(int socket, char *username);
 *
 * Parameters:
 *   x_socket - Socket of player X (the challenger)
 *   o_socket - Socket of player O (the challenged)
 *   game_id  - The ID of the newly created game
 *****************************************************************************/
void send_game_started(int x_socket, int o_socket, int game_id) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() for building packets */
    /* See the detailed packet format and implementation steps above */

    fprintf(stderr, "ERROR: send_game_started() not implemented\n");
}

/*****************************************************************************
 * TODO: handle_move - Process Flag 30 (player move)
 *
 * This function processes a move from a player, validates it, updates the game
 * state, and notifies both players of the result.
 *
 * INCOMING PACKET FORMAT (Flag 30):
 * +------+---------+----------+
 * | Flag | Game ID | Position |
 * +------+---------+----------+
 * | 1 B  | 1 B     | 1 B      |
 * +------+---------+----------+
 *   [0] = FLAG_MOVE (30)
 *   [1] = game_id
 *   [2] = position (1-9, corresponding to board positions)
 *
 * Board position numbering:
 *   1 | 2 | 3
 *   ---------
 *   4 | 5 | 6
 *   ---------
 *   7 | 8 | 9
 *
 * EXAMPLE - Player moves to position 5 in game 3:
 *   [1E][03][05]
 *    ^   ^   ^-- Position: 5 (center)
 *    |   +------ Game ID: 3
 *    +---------- Flag 30
 *
 * OUTGOING PACKETS:
 *
 * Success (Flag 31) - Board update (sent to BOTH players):
 * +------+---------+----------+----------+-------+---------------+
 * | Flag | Game ID | Position | Who Moved| Board | Current Turn  |
 * +------+---------+----------+----------+-------+---------------+
 * | 1 B  | 1 B     | 1 B      | 1 B      | 9 B   | 1 B           |
 * +------+---------+----------+----------+-------+---------------+
 *   [0]     = FLAG_BOARD_UPDATE (31)
 *   [1]     = game_id
 *   [2]     = position just played (1-9)
 *   [3]     = who moved (SYMBOL_X=1 or SYMBOL_O=0)
 *   [4..12] = board state (9 bytes: CELL_EMPTY=0, CELL_X=1, CELL_O=2)
 *   [13]    = whose turn next (SYMBOL_X=1 or SYMBOL_O=0)
 *
 * Error (Flag 32) - Invalid move (sent only to requesting player):
 * +------+------------+
 * | Flag | Error Code |
 * +------+------------+
 * | 1 B  | 1 B        |
 * +------+------------+
 *   [0] = FLAG_MOVE_INVALID (32)
 *   [1] = error_code (see below)
 *
 * ERROR CODES:
 *   0 = Not your turn
 *   1 = Position already occupied
 *   2 = Invalid position (not 1-9)
 *   3 = Not in a game / generic error
 *
 * Game Over (Flag 33) - sent to BOTH players if game ends:
 * +------+---------+--------+-------+
 * | Flag | Game ID | Result | Board |
 * +------+---------+--------+-------+
 * | 1 B  | 1 B     | 1 B    | 9 B   |
 * +------+---------+--------+-------+
 *   [0]    = FLAG_GAME_OVER (33)
 *   [1]    = game_id
 *   [2]    = result (see result codes below)
 *   [3..11] = final board state (9 bytes)
 *
 * RESULT CODES:
 *   0 = Draw
 *   1 = X won
 *   2 = O won
 *   3 = X forfeited
 *   4 = O forfeited
 *   5 = X disconnected
 *   6 = O disconnected
 *
 * IMPLEMENTATION STEPS:
 * 1. Parse the move packet
 *    - Validate: if (len < 3) return
 *    - uint8_t game_id = buffer[1]
 *    - uint8_t position = buffer[2]
 *
 * 2. Validate player is in this game
 *    - if (game_get_by_socket(socket) != game_id)
 *    - Send Flag 32 with error code 3
 *    - return
 *
 * 3. Attempt the move using the game module
 *    - int result = game_make_move(game_id, socket, position)
 *
 * 4. Handle the result
 *    a. If result == 0 (success):
 *       - Get who moved: int symbol = game_get_symbol(game_id, socket)
 *       - Send board update: send_board_update(game_id, position, symbol)
 *       - Check for game end:
 *         - int winner = game_check_winner(game_id)
 *         - if (winner == CELL_X): send_game_over(game_id, RESULT_X_WON)
 *         - else if (winner == CELL_O): send_game_over(game_id, RESULT_O_WON)
 *         - else if (game_is_draw(game_id)): send_game_over(game_id, RESULT_DRAW)
 *
 *    b. If result < 0 (error):
 *       - Build Flag 32 packet:
 *         - buffer[0] = FLAG_MOVE_INVALID
 *         - Map game module error code to protocol error code:
 *           - result == -2: buffer[1] = 0 (not your turn)
 *           - result == -3: buffer[1] = 2 (invalid position)
 *           - result == -4: buffer[1] = 1 (position occupied)
 *           - else: buffer[1] = 3 (generic error)
 *       - sendPDU(socket, buffer, 2)
 *
 * GAME MODULE FUNCTIONS YOU'LL NEED:
 *   int game_get_by_socket(int socket);
 *   int game_make_move(int game_id, int socket, int position);
 *   int game_get_symbol(int game_id, int socket);
 *   int game_check_winner(int game_id);
 *   int game_is_draw(int game_id);
 *
 * Parameters:
 *   socket - The socket of the player making the move
 *   buffer - The received packet
 *   len    - Length of the received packet
 *****************************************************************************/
void handle_move(int socket, uint8_t *buffer, int len) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() for parsing */
    /* Use send_board_update() and send_game_over() for notifications */
    /* See the detailed packet formats and implementation steps above */

    fprintf(stderr, "ERROR: handle_move() not implemented\n");
}

/*****************************************************************************
 * TODO: send_board_update - Send Flag 31 to both players
 *
 * This function sends the updated board state to both players after a valid move.
 *
 * PACKET FORMAT (Flag 31):
 * +------+---------+----------+----------+-------+---------------+
 * | Flag | Game ID | Position | Who Moved| Board | Current Turn  |
 * +------+---------+----------+----------+-------+---------------+
 * | 1 B  | 1 B     | 1 B      | 1 B      | 9 B   | 1 B           |
 * +------+---------+----------+----------+-------+---------------+
 *   [0]     = FLAG_BOARD_UPDATE (31)
 *   [1]     = game_id
 *   [2]     = position just played (1-9)
 *   [3]     = who moved (SYMBOL_X=1 or SYMBOL_O=0)
 *   [4..12] = board state (9 bytes)
 *   [13]    = whose turn next (SYMBOL_X=1 or SYMBOL_O=0)
 *
 * Board encoding:
 *   CELL_EMPTY = 0
 *   CELL_X     = 1
 *   CELL_O     = 2
 *
 * EXAMPLE - X plays position 5, now O's turn:
 *   [1F][03][05][01][00 00 00 00 01 00 00 00 00][00]
 *    ^   ^   ^   ^   ^------------------------ Board: [empty,empty,empty,empty,X,empty,empty,empty,empty]
 *    |   |   |   |                              ^--- Next turn: O (0)
 *    |   |   |   +------------------------------- Who moved: X (1)
 *    |   |   +----------------------------------- Position: 5
 *    |   +--------------------------------------- Game ID: 3
 *    +------------------------------------------- Flag 31
 *
 * IMPLEMENTATION STEPS:
 * 1. Get game state from game module
 *    - Declare: uint8_t board[9];
 *    - Call: game_get_board(game_id, board)
 *    - Call: int current_turn = game_get_current_turn(game_id)
 *
 * 2. Get both player sockets
 *    - int x_socket = game_get_x_socket(game_id)
 *    - int o_socket = game_get_o_socket(game_id)
 *    - if (x_socket < 0 || o_socket < 0) return (game no longer exists)
 *
 * 3. Build the packet
 *    - buffer[0] = FLAG_BOARD_UPDATE
 *    - buffer[1] = (uint8_t)game_id
 *    - buffer[2] = (uint8_t)position
 *    - buffer[3] = (uint8_t)who_moved
 *    - memcpy(buffer + 4, board, 9)
 *    - buffer[13] = (uint8_t)current_turn
 *
 * 4. Send to both players
 *    - sendPDU(x_socket, buffer, 14)
 *    - sendPDU(o_socket, buffer, 14)
 *
 * GAME MODULE FUNCTIONS YOU'LL NEED:
 *   void game_get_board(int game_id, uint8_t *board);
 *   int game_get_current_turn(int game_id);
 *   int game_get_x_socket(int game_id);
 *   int game_get_o_socket(int game_id);
 *
 * Parameters:
 *   game_id   - The game being updated
 *   position  - The position that was just played (1-9)
 *   who_moved - The symbol of who made the move (SYMBOL_X or SYMBOL_O)
 *****************************************************************************/
void send_board_update(int game_id, int position, int who_moved) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() for building packets */
    /* See the detailed packet format and implementation steps above */

    fprintf(stderr, "ERROR: send_board_update() not implemented\n");
}

/*****************************************************************************
 * TODO: send_game_over - Send Flag 33 to both players and clean up game
 *
 * This function notifies both players that the game has ended and performs cleanup.
 *
 * PACKET FORMAT (Flag 33):
 * +------+---------+--------+-------+
 * | Flag | Game ID | Result | Board |
 * +------+---------+--------+-------+
 * | 1 B  | 1 B     | 1 B    | 9 B   |
 * +------+---------+--------+-------+
 *   [0]    = FLAG_GAME_OVER (33)
 *   [1]    = game_id
 *   [2]    = result code
 *   [3..11] = final board state (9 bytes)
 *
 * RESULT CODES:
 *   0 = Draw
 *   1 = X won
 *   2 = O won
 *   3 = X forfeited
 *   4 = O forfeited
 *   5 = X disconnected
 *   6 = O disconnected
 *
 * EXAMPLE - X wins game 3:
 *   [21][03][01][01 01 01 02 02 00 00 00 00]
 *    ^   ^   ^   ^--------------------------- Board: X won top row
 *    |   |   +------------------------------- Result: 1 (X won)
 *    |   +----------------------------------- Game ID: 3
 *    +--------------------------------------- Flag 33
 *
 * IMPLEMENTATION STEPS:
 * 1. Get game state
 *    - Declare: uint8_t board[9];
 *    - Call: game_get_board(game_id, board)
 *
 * 2. Get both player sockets and usernames
 *    - int x_socket = game_get_x_socket(game_id)
 *    - int o_socket = game_get_o_socket(game_id)
 *    - If either is < 0:
 *      - game_destroy(game_id)
 *      - return
 *    - Declare: char x_username[101], o_username[101];
 *    - Call: users_get_username(x_socket, x_username)
 *    - Call: users_get_username(o_socket, o_username)
 *
 * 3. Build the packet
 *    - buffer[0] = FLAG_GAME_OVER
 *    - buffer[1] = (uint8_t)game_id
 *    - buffer[2] = (uint8_t)result
 *    - memcpy(buffer + 3, board, 9)
 *
 * 4. Send to both players
 *    - sendPDU(x_socket, buffer, 12)
 *    - sendPDU(o_socket, buffer, 12)
 *
 * 5. Update user states back to available
 *    - users_set_state(x_username, USER_AVAILABLE)
 *    - users_set_state(o_username, USER_AVAILABLE)
 *
 * 6. Destroy the game
 *    - game_destroy(game_id)
 *
 * USERS MODULE FUNCTIONS YOU'LL NEED:
 *   int users_get_username(int socket, char *username);
 *   void users_set_state(const char *username, UserState state);
 *
 * GAME MODULE FUNCTIONS YOU'LL NEED:
 *   void game_get_board(int game_id, uint8_t *board);
 *   int game_get_x_socket(int game_id);
 *   int game_get_o_socket(int game_id);
 *   void game_destroy(int game_id);
 *
 * Parameters:
 *   game_id - The game that is ending
 *   result  - The result code (see list above)
 *****************************************************************************/
void send_game_over(int game_id, int result) {
    /* TODO: Implement this function */
    /* Follow the pattern from handle_initial_connection() for building packets */
    /* Don't forget to clean up both user states and the game! */
    /* See the detailed packet format and implementation steps above */

    fprintf(stderr, "ERROR: send_game_over() not implemented\n");
}

/*****************************************************************************
 * TODO: handle_disconnect - Clean up when a client disconnects
 *
 * This function handles all cleanup when a client disconnects, either
 * gracefully or due to an error. It must handle game state, user state,
 * and notify opponents if necessary.
 *
 * DISCONNECT SCENARIOS:
 * 1. Player not in a game - simple cleanup
 * 2. Player in a game - notify opponent and end game
 *
 * IMPLEMENTATION STEPS:
 * 1. Get the disconnecting user's information
 *    - Declare: char username[101];
 *    - Declare: int game_id;
 *    - Call: users_get_username(socket, username)
 *    - If successful: Print "Player %s disconnected\n"
 *
 * 2. Check if player is in a game
 *    - Call: game_id = game_get_by_socket(socket);
 *    - If game_id >= 0, the player was in a game
 *
 * 3. If in a game, notify opponent and clean up
 *    a. Get opponent's socket:
 *       - int opponent = game_get_opponent(game_id, socket);
 *
 *    b. If opponent >= 0, send game over message:
 *       - Get your symbol: int symbol = game_get_symbol(game_id, socket)
 *       - Get the board: uint8_t board[9]; game_get_board(game_id, board);
 *       - Build Flag 33 packet:
 *         - buffer[0] = FLAG_GAME_OVER
 *         - buffer[1] = (uint8_t)game_id
 *         - buffer[2] = (symbol == SYMBOL_X) ? RESULT_X_DISCONN : RESULT_O_DISCONN
 *         - memcpy(buffer + 3, board, 9)
 *       - Send: sendPDU(opponent, buffer, 12)
 *
 *    c. Set opponent back to available:
 *       - Declare: char opp_username[101];
 *       - Get opponent username: users_get_username(opponent, opp_username)
 *       - Update state: users_set_state(opp_username, USER_AVAILABLE)
 *
 *    d. Destroy the game:
 *       - game_destroy(game_id)
 *
 * 4. Remove from users table
 *    - users_remove_by_socket(socket)
 *
 * 5. Close the socket
 *    - close(socket)
 *
 * 6. Remove from poll array
 *    - Move the last element to this position: pfds[index] = pfds[*num_fds - 1]
 *    - Decrement count: (*num_fds)--
 *    - Note: This moves pfds[*num_fds-1] into pfds[index], so if you're
 *      calling this in a loop, you may need to re-check pfds[index]
 *
 * RESULT CODES FOR FLAG 33:
 *   RESULT_X_DISCONN = 5
 *   RESULT_O_DISCONN = 6
 *
 * USERS MODULE FUNCTIONS YOU'LL NEED:
 *   int users_get_username(int socket, char *username);
 *   void users_set_state(const char *username, UserState state);
 *   void users_remove_by_socket(int socket);
 *
 * GAME MODULE FUNCTIONS YOU'LL NEED:
 *   int game_get_by_socket(int socket);
 *   int game_get_opponent(int game_id, int socket);
 *   int game_get_symbol(int game_id, int socket);
 *   void game_get_board(int game_id, uint8_t *board);
 *   void game_destroy(int game_id);
 *
 * Parameters:
 *   socket  - The socket being disconnected
 *   pfds    - Array of poll file descriptors
 *   num_fds - Pointer to number of active file descriptors
 *   index   - Index in pfds array for this client
 *****************************************************************************/
void handle_disconnect(int socket, struct pollfd *pfds, int *num_fds, int index) {
    /* TODO: Implement this function */
    /* See the function header above for detailed implementation steps */

    fprintf(stderr, "ERROR: handle_disconnect() not implemented\n");
}
