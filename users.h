/*****************************************************************************
 * users.h - Username table management (server-side)
 *
 * This module maintains a table mapping usernames to socket descriptors.
 * All access to the username table must go through these functions.
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#ifndef USERS_H
#define USERS_H

#include <stdint.h>

/* User states */
typedef enum {
    USER_AVAILABLE,   /* Can start or join games */
    USER_IN_GAME      /* Currently playing a game */
} UserState;

/*****************************************************************************
 * users_init - Initialize the username table
 *
 * Must be called before any other functions in this module.
 *
 * TODO: Initialize your dynamic data structure here
 *****************************************************************************/
void users_init(void);

/*****************************************************************************
 * users_add - Add a new user to the table
 *
 * Parameters:
 *   username - The username to add (null-terminated)
 *   socket   - The socket descriptor for this user
 *
 * Returns:
 *   0 on success
 *   -1 if username already exists
 *   -2 on memory allocation failure
 *
 * TODO: Implement this function. You must:
 *   1. Check if username already exists (return -1 if it does)
 *   2. Allocate memory for a new entry
 *   3. Copy the username and store the socket
 *   4. Add the entry to your data structure
 *   5. Initialize the user state to USER_AVAILABLE
 *****************************************************************************/
int users_add(const char *username, int socket);

/*****************************************************************************
 * users_remove - Remove a user from the table
 *
 * Parameters:
 *   username - The username to remove
 *
 * Returns:
 *   0 on success
 *   -1 if username not found
 *
 * TODO: Implement this function. You must:
 *   1. Find the user in your data structure
 *   2. Remove them from the structure
 *   3. Free any allocated memory
 *****************************************************************************/
int users_remove(const char *username);

/*****************************************************************************
 * users_remove_by_socket - Remove a user by socket descriptor
 *
 * Parameters:
 *   socket - The socket descriptor
 *
 * Returns:
 *   0 on success
 *   -1 if socket not found
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_remove_by_socket(int socket);

/*****************************************************************************
 * users_exists - Check if a username exists
 *
 * Parameters:
 *   username - The username to check
 *
 * Returns:
 *   1 if exists
 *   0 if does not exist
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_exists(const char *username);

/*****************************************************************************
 * users_get_socket - Get the socket for a username
 *
 * Parameters:
 *   username - The username to look up
 *
 * Returns:
 *   Socket descriptor on success
 *   -1 if username not found
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_get_socket(const char *username);

/*****************************************************************************
 * users_get_username - Get the username for a socket
 *
 * Parameters:
 *   socket   - The socket to look up
 *   username - Buffer to store the username (must be at least 101 bytes)
 *
 * Returns:
 *   0 on success
 *   -1 if socket not found
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_get_username(int socket, char *username);

/*****************************************************************************
 * users_set_state - Set the state for a user
 *
 * Parameters:
 *   username - The username
 *   state    - The new state
 *
 * Returns:
 *   0 on success
 *   -1 if username not found
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_set_state(const char *username, UserState state);

/*****************************************************************************
 * users_get_state - Get the state for a user
 *
 * Parameters:
 *   username - The username
 *
 * Returns:
 *   The user's state
 *   USER_AVAILABLE if username not found (safe default)
 *
 * TODO: Implement this function
 *****************************************************************************/
UserState users_get_state(const char *username);

/*****************************************************************************
 * users_count - Get the total number of users
 *
 * Returns:
 *   Number of users in the table
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_count(void);

/*****************************************************************************
 * users_get_all - Get all usernames
 *
 * Parameters:
 *   usernames - Array of string pointers (caller allocates)
 *   max_users - Maximum number of usernames to return
 *
 * Returns:
 *   Number of usernames copied into the array
 *
 * Note: Caller must allocate space for each username string (101 bytes each)
 *
 * TODO: Implement this function
 *****************************************************************************/
int users_get_all(char **usernames, int max_users);

/*****************************************************************************
 * users_cleanup - Free all memory used by the username table
 *
 * TODO: Implement this function. Free ALL allocated memory.
 *****************************************************************************/
void users_cleanup(void);

#endif /* USERS_H */
