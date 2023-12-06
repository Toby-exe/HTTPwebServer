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
