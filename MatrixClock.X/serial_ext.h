/* 
 * File:   serial_ext.h
 * Author: smiller
 *
 * Created on December 5, 2015, 2:49 AM
 */

#ifndef SERIAL_EXT_H
#define	SERIAL_EXT_H

//#include "pt_cornell_1_2.h"

// Setup the protothread structures that are used for the put and get threads.
// These should not be used for any other thread besides spawning
// PT_GetSerialBuffer and PutSerialBuffer
static struct pt pt_get, pt_put;

// Printf using PT PutSerialBuffer
#define pt_printf(fmt, ...) \
    do { snprintf(PT_send_buffer, max_chars, fmt, __VA_ARGS__);\
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt//_put) );\
       } while(0)

// Printf with terminating newline using PT PutSerialBuffer
#define pt_printfl(fmt, ...) \
    do { snprintf(PT_send_buffer, max_chars-2, fmt, __VA_ARGS__); \
         strcat(PT_send_buffer, "\n\r"); \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

// Print simple string using PT PutSerialBuffer
#define pt_print(str) \
    do { strncpy(PT_send_buffer, (str), max_chars-1); \
         PT_send_buffer[max_chars-1] = '\0'; \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

// Print simple string with terminating newline using PT PutSerialBuffer
#define pt_printl(str) \
    do { strncpy(PT_send_buffer, (str), max_chars-3); \
         PT_send_buffer[max_chars-3] = '\0'; \
         strcat(PT_send_buffer, "\n\r"); \
         PT_SPAWN( pt, &pt_put, PutSerialBuffer(&pt_put) ); \
       } while(0)

#endif	/* SERIAL_EXT_H */

