/**
 * @file files.c
 * @authors Tobias Wondwossen, Jayden Mingle
 *
 * @date 2023-12-06
 */

#include "files.h"

/**
 * @brief Saves data to a file
 *
 * This function takes a file path and a data string as input. It opens the file in write mode, writes the data to 
 * the file, and then closes the file. If there is an error during the process, it prints an error message and returns -1.
 * 
 * @param[in] file_path The path to the file
 * @param[in] data The data to be written to the file
 * @return 0 if the data was saved successfully, -1 otherwise
 */
int save_file(char *file_path, const char *data)
{
    FILE *fp;
    char file_path_with_dir[1024] = "../public";
    strcat(file_path_with_dir, file_path);
    fp = fopen(file_path_with_dir, "w");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        return -1;
    }

    fputs(data, fp);
    fclose(fp);

    return 0;
}

/**
 * @brief Save a JSON object to a file
 *
 * This function takes a file path and a JSON string as input and appends the
 * JSON object to an existing or new JSON array in the file. It uses the cJSON
 * library to parse and print JSON data.
 *
 * @param[in] file_path The relative path of the file to save the JSON object to
 * @param[in] data The JSON string to be saved
 * @return 0 if the operation was successful, -1 otherwise
 */
int save_json(char *file_path, const char *data)
{
    FILE *fp;
    char file_path_with_dir[1024] = "../public";
    strcat(file_path_with_dir, file_path);
    fp = fopen(file_path_with_dir, "r");
    cJSON *json;

    if (fp == NULL)
    {
        // if the file doesn't exist, create a new json array
        json = cJSON_CreateArray();
    }
    else
    {
        // if the file exists, read its content
        char buffer[MAX_BODY_SIZE];
        size_t bytes_read = fread(buffer, 1, MAX_BODY_SIZE, fp);
        fclose(fp);

        if (bytes_read == 0)
        {
            // if the file is empty, create a new json array
            json = cJSON_CreateArray();
        }
        else
        {
            // if the file is not empty, parse the existing json array
            json = cJSON_Parse(buffer);
        }
    }

    // create a new json object from the request body
    cJSON *new_object = cJSON_Parse(data);

    // add the new object to the json array
    cJSON_AddItemToArray(json, new_object);

    printf("tinyserver: json is %s\n", cJSON_Print(json));

    // write the updated json array back to the file
    fp = fopen(file_path_with_dir, "w");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return -1;
    }
    char *json_string = cJSON_Print(json);
    fputs(json_string, fp);
    free(json_string);
    cJSON_Delete(json);
    fclose(fp);

    return 0;
}


/**
 * @brief Checks if a file exists
 *
 * This function takes a path and a root directory as input. It constructs the full path to the file and checks 
 * if the file exists. If the file exists, it returns true; otherwise, it returns false.
 * 
 * @param[in] path The path to the file
 * @param[in] root_dir The root directory
 * @return true if the file exists, false otherwise
 */
bool file_exists(char *path, char *root_dir)
{
    char full_path[BUF_SIZE];
    strcpy(full_path, root_dir);
    strcat(full_path, path);

    printf("checking if file exists at: %s\n", full_path);

    if (access(full_path, F_OK) == -1)
    {
        printf("File does not exist\n");
        return false;
    }
    else
    {
        return true;
    }
}