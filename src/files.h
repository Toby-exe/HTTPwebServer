/**
 * @file files.h
 * @brief A library for handling file operations in a web server program
 * @authors Tobias Wondwossen, Jayden Mingle
 * 
 * Details: 
 * - This library provides the necessary function prototypes for handling file operations in a web server program. It includes the definitions of various constants such as MAX_BODY_SIZE and BUF_SIZE.
 * - Function prototypes for saving JSON and other files, and checking if a file exists are provided.
 * 
 * Constants:
 * - MAX_BODY_SIZE: Represents the maximum size of the body of an HTTP request or response.
 * - BUF_SIZE: Represents the size of the buffer used for reading from and writing to files.
 * 
 * Function Prototypes:
 * - Saving JSON files
 * - Saving other types of files
 * - Checking if a file exists
 * 
 * Assumptions/Limitations: 
 * - This library assumes that the maximum size of the body of an HTTP request or response is MAX_BODY_SIZE, and the size of the buffer used for reading from and writing to files is BUF_SIZE.
 * - It does not handle cases where these limits are exceeded.
 * 
 * @date 2023-12-06
 */

#ifndef FILES_H
#define FILES_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cJSON.h"

#define MAX_BODY_SIZE 1000000
#define BUF_SIZE 1024

int save_json(char *file_path, const char *data);
int save_file(char *file_path, const char *data);
bool file_exists(char *path, char *root_dir);

#endif 