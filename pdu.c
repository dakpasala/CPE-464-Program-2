/*****************************************************************************
 * pdu.c - Protocol Data Unit helper functions implementation
 *
 * Author: Paul Schmitt
 * CPE 464 - Assignment 2
 *****************************************************************************/

#include "pdu.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

/*****************************************************************************
 * sendPDU - Send a Protocol Data Unit with length prefix
 *****************************************************************************/
int sendPDU(int socket, uint8_t *buffer, int length) {
    uint16_t pdu_length;
    uint8_t length_bytes[2];
    int sent = 0;
    int bytes_sent;

    /* Validate parameters */
    if (buffer == NULL || length < 0) {
        return -1;
    }

    /* Check if length would overflow uint16_t */
    if (length > 65533) {  /* Max: 65535 - 2 bytes for length field */
        fprintf(stderr, "sendPDU: packet too large (%d bytes)\n", length);
        return -1;
    }

    /* Calculate total PDU length (data + 2-byte length field) */
    pdu_length = length + 2;

    /* Convert length to network byte order */
    pdu_length = htons(pdu_length);
    memcpy(length_bytes, &pdu_length, 2);

    /* Send the 2-byte length field first */
    sent = 0;
    while (sent < 2) {
        bytes_sent = send(socket, length_bytes + sent, 2 - sent, 0);
        if (bytes_sent < 0) {
            perror("sendPDU: error sending length");
            return -1;
        }
        if (bytes_sent == 0) {
            /* Connection closed */
            return -1;
        }
        sent += bytes_sent;
    }

    /* Send the actual data */
    sent = 0;
    while (sent < length) {
        bytes_sent = send(socket, buffer + sent, length - sent, 0);
        if (bytes_sent < 0) {
            perror("sendPDU: error sending data");
            return -1;
        }
        if (bytes_sent == 0) {
            /* Connection closed */
            return -1;
        }
        sent += bytes_sent;
    }

    /* Return total bytes sent (length field + data) */
    return length + 2;
}

/*****************************************************************************
 * recvPDU - Receive a Protocol Data Unit with length prefix
 *****************************************************************************/
int recvPDU(int socket, uint8_t *buffer, int max_length) {
    uint16_t pdu_length;
    uint8_t length_bytes[2];
    int bytes_received;
    int data_length;

    /* Validate parameters */
    if (buffer == NULL || max_length <= 0) {
        return -1;
    }

    /* First, receive the 2-byte length field */
    bytes_received = recv(socket, length_bytes, 2, MSG_WAITALL);

    if (bytes_received == 0) {
        /* Connection closed by remote side */
        return 0;
    }

    if (bytes_received < 0) {
        perror("recvPDU: error receiving length");
        return -1;
    }

    if (bytes_received != 2) {
        fprintf(stderr, "recvPDU: incomplete length field (%d/2 bytes)\n",
                bytes_received);
        return -1;
    }

    /* Convert length from network byte order to host byte order */
    memcpy(&pdu_length, length_bytes, 2);
    pdu_length = ntohs(pdu_length);

    /* Validate PDU length */
    if (pdu_length < 2) {
        fprintf(stderr, "recvPDU: invalid PDU length (%d)\n", pdu_length);
        return -1;
    }

    /* Calculate data length (total length - 2-byte length field) */
    data_length = pdu_length - 2;

    /* Check if buffer is large enough */
    if (data_length > max_length) {
        fprintf(stderr, "recvPDU: PDU too large (%d bytes, buffer is %d)\n",
                data_length, max_length);
        return -1;
    }

    /* Receive the actual data */
    bytes_received = recv(socket, buffer, data_length, MSG_WAITALL);

    if (bytes_received == 0) {
        /* Connection closed by remote side */
        return 0;
    }

    if (bytes_received < 0) {
        perror("recvPDU: error receiving data");
        return -1;
    }

    if (bytes_received != data_length) {
        fprintf(stderr, "recvPDU: incomplete data (%d/%d bytes)\n",
                bytes_received, data_length);
        return -1;
    }

    /* Return number of data bytes received (NOT including length field) */
    return data_length;
}
