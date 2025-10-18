/*****************************************************************************
 * users.c - Username table management implementation
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#include "users.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Define your data structure here
 *
 * Example for linked list:
 * typedef struct UserNode {
 *     char username[101];
 *     int socket;
 *     UserState state;
 *     struct UserNode *next;
 * } UserNode;
 *
 * static UserNode *user_list = NULL;
 *
 * Example for dynamic array:
 * typedef struct {
 *     char username[101];
 *     int socket;
 *     UserState state;
 * } User;
 *
 * static User *user_array = NULL;
 * static int user_count = 0;
 * static int user_capacity = 0;
 */

/* TODO: Define your global variables here */


/*****************************************************************************
 * users_init - Initialize the username table
 *****************************************************************************/
void users_init(void) {
    /* TODO: Initialize your data structure
     * For linked list: set head to NULL
     * For dynamic array: allocate initial capacity
     * For hash table: allocate and initialize buckets
     */
}

/*****************************************************************************
 * users_add - Add a new user to the table
 *****************************************************************************/
int users_add(const char *username, int socket) {
    /* TODO: Implement this function
     *
     * Steps:
     * 1. Check if username already exists (use users_exists or search manually)
     *    If it exists, return -1
     *
     * 2. Allocate memory for a new user entry
     *    If allocation fails, return -2
     *
     * 3. Copy username (use strncpy or strcpy)
     *
     * 4. Store socket descriptor
     *
     * 5. Set state to USER_AVAILABLE
     *
     * 6. Add to your data structure:
     *    - Linked list: add to head or tail
     *    - Dynamic array: check capacity, realloc if needed, add to end
     *    - Hash table: hash username and add to appropriate bucket
     *
     * 7. Return 0 on success
     */

    return -2;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_remove - Remove a user from the table
 *****************************************************************************/
int users_remove(const char *username) {
    /* TODO: Implement this function
     *
     * Steps:
     * 1. Find the user with matching username
     *
     * 2. If not found, return -1
     *
     * 3. Remove from data structure:
     *    - Linked list: adjust pointers to skip node
     *    - Dynamic array: shift elements or swap with last and decrement count
     *    - Hash table: remove from bucket
     *
     * 4. Free allocated memory for the user entry
     *
     * 5. Return 0
     */

    return -1;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_remove_by_socket - Remove a user by socket descriptor
 *****************************************************************************/
int users_remove_by_socket(int socket) {
    /* TODO: Implement this function
     *
     * Similar to users_remove, but search by socket instead of username
     */

    return -1;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_exists - Check if a username exists
 *****************************************************************************/
int users_exists(const char *username) {
    /* TODO: Implement this function
     *
     * Search your data structure for the username
     * Return 1 if found, 0 if not found
     *
     * Use strcmp() to compare strings
     */

    return 0;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_get_socket - Get the socket for a username
 *****************************************************************************/
int users_get_socket(const char *username) {
    /* TODO: Implement this function
     *
     * Search your data structure for the username
     * Return the socket descriptor if found
     * Return -1 if not found
     */

    return -1;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_get_username - Get the username for a socket
 *****************************************************************************/
int users_get_username(int socket, char *username) {
    /* TODO: Implement this function
     *
     * Search your data structure for the socket
     * If found, copy the username to the buffer (use strcpy)
     * Return 0 if found, -1 if not found
     *
     * Make sure the buffer is at least 101 bytes (you can assume it is)
     */

    return -1;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_set_state - Set the state for a user
 *****************************************************************************/
int users_set_state(const char *username, UserState state) {
    /* TODO: Implement this function
     *
     * Find the user and update their state
     * Return 0 on success, -1 if not found
     */

    return -1;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_get_state - Get the state for a user
 *****************************************************************************/
UserState users_get_state(const char *username) {
    /* TODO: Implement this function
     *
     * Find the user and return their state
     * Return USER_AVAILABLE if not found (safe default)
     */

    return USER_AVAILABLE;  /* Placeholder - always returns this now */
}

/*****************************************************************************
 * users_count - Get the total number of users
 *****************************************************************************/
int users_count(void) {
    /* TODO: Implement this function
     *
     * For linked list: traverse and count nodes
     * For dynamic array: return user_count variable
     * For hash table: count entries across all buckets
     */

    return 0;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_get_all - Get all usernames
 *****************************************************************************/
int users_get_all(char **usernames, int max_users) {
    /* TODO: Implement this function
     *
     * Iterate through your data structure
     * Copy each username into usernames array:
     *   strcpy(usernames[i], current_user->username);
     *
     * Stop when you've copied max_users or reached the end
     * Return the number of usernames copied
     */

    return 0;  /* Placeholder - remove this when implementing */
}

/*****************************************************************************
 * users_cleanup - Free all memory used by the username table
 *****************************************************************************/
void users_cleanup(void) {
    /* TODO: Implement this function
     *
     * Free ALL dynamically allocated memory
     *
     * For linked list:
     *   - Traverse list, free each node
     *   - Set head to NULL
     *
     * For dynamic array:
     *   - free(user_array)
     *   - Set user_array to NULL, count to 0
     *
     * For hash table:
     *   - Free all entries in all buckets
     *   - Free the bucket array
     */
}
