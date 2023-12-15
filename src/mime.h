/**
 * @file mime.h
 * @brief A library for getting the MIME type of a file
 * @authors Jayden Mingle
 * 
 * Details:
 * Before the server starts accepting connections, a hash table is set up that
 * holds the necessary file extension -> MIME type conversions needed by a client
 * like Google Chrome. If an unknown file extension is given to this
 * hash table, it defaults the mime type to "application/octet-stream". This MIME type
 * is generic and can be used for any binary file. It preserves the file's
 * contents without making assumptions about its type.
 * 
 * Assumptions/Limitations:
 * Doesn't support every single MIME type (only the common types that we're likely
 * to encounter while running the server). Operates on a filename string that we assume
 * has been allocated by the caller and will be freed by the caller as well.
 * 
 * @date 2023-12-2
 */
#ifndef _MIME_H_
#define _MIME_H_

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"

#define HT_SIZE 16
#define DEFAULT_MIME_TYPE "application/octet-stream"

extern HashTable *ext_to_mime;

/* ----------{ FUNCTION PROTOTYPES }---------- */

extern void create_mime_db();
extern void destroy_mime_db();
extern char *get_mime_type(char *filename);

#endif