/*****************************************************************************
 * pdu.h - Protocol Data Unit helper functions
 *
 * These functions handle sending and receiving packets with a 2-byte
 * length prefix in network byte order.
 *
 * All packets in this protocol start with a 2-byte length field that
 * specifies the total length of the packet INCLUDING the 2-byte length
 * field itself.
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#ifndef PDU_H
#define PDU_H

#include <stdint.h>

/*****************************************************************************
 * sendPDU - Send a Protocol Data Unit with length prefix
 *
 * This function prepends a 2-byte length field (in network byte order) to
 * the data buffer and sends the entire packet over the socket.
 *
 * Parameters:
 *   socket - The socket descriptor to send on
 *   buffer - Pointer to the data to send (does NOT include length prefix)
 *   length - The length of the data in buffer (NOT including the 2-byte prefix)
 *
 * Returns:
 *   On success: Total bytes sent (length + 2)
 *   On error: -1
 *
 * Notes:
 *   - The 2-byte length field contains: length + 2 (includes itself)
 *   - The length is converted to network byte order before sending
 *   - This function handles partial sends by looping until all data is sent
 *
 * Example:
 *   uint8_t packet[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
 *   int result = sendPDU(socket_fd, packet, 10);
 *   // Sends: [0x00, 0x0C, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
 *   //         ^^^^^^^^^^ 2-byte length = 12 in network order
 *****************************************************************************/
int sendPDU(int socket, uint8_t *buffer, int length);

/*****************************************************************************
 * recvPDU - Receive a Protocol Data Unit with length prefix
 *
 * This function first reads the 2-byte length prefix (in network byte order),
 * then reads exactly that many more bytes from the socket.
 *
 * Parameters:
 *   socket     - The socket descriptor to receive from
 *   buffer     - Pointer to buffer where received data will be stored
 *   max_length - Maximum size of buffer (to prevent overflow)
 *
 * Returns:
 *   On success: Number of bytes received (NOT including the 2-byte length prefix)
 *   On disconnect: 0 (remote side closed connection)
 *   On error: -1
 *
 * Notes:
 *   - First reads 2 bytes for length field
 *   - Then reads (length - 2) more bytes of actual data
 *   - Uses MSG_WAITALL to ensure entire PDU is received before returning
 *   - If the PDU is larger than max_length, returns -1 (error)
 *   - Does NOT null-terminate the buffer
 *
 * Example:
 *   uint8_t buffer[1024];
 *   int bytes_received = recvPDU(socket_fd, buffer, sizeof(buffer));
 *   if (bytes_received > 0) {
 *       // Process the packet in buffer
 *       uint8_t flag = buffer[0];  // First byte is usually the flag
 *   }
 *****************************************************************************/
int recvPDU(int socket, uint8_t *buffer, int max_length);

#endif /* PDU_H */
