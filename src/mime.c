#include "mime.h"

HashTable *ext_to_mime;

/**
 * @brief converts all uppercase characters in a string to lowercase
 * 
 * @param[in] str The string being converted
 */
void strlower(char *str)
{
    for (char *c = str; *c != '\0'; c++) {
        *c = (char)tolower((unsigned char)*c);
    }
}

/**
 * @brief creates a new hash table containing a list of file extension to MIME type
 *        mappings
 * 
 * Details: Only stores some basic conversions that we use for this server. 
 *          There's no way to know every MIME type and file extension in existence
 *          and many aren't even supported by browsers like Google Chrome.
 */
void create_mime_db()
{
    ext_to_mime = Hashtable_create(HT_SIZE, NULL);
    
    /* key = file extension, value = mime type */
    Hashtable_put(ext_to_mime, "html", "text/html");
    Hashtable_put(ext_to_mime, "htm", "text/html");
    Hashtable_put(ext_to_mime, "jpeg", "image/jpg");
    Hashtable_put(ext_to_mime, "jpg", "image/jpg");
    Hashtable_put(ext_to_mime, "css", "text/css");
    Hashtable_put(ext_to_mime, "js", "application/javascript");
    Hashtable_put(ext_to_mime, "json", "application/json");
    Hashtable_put(ext_to_mime, "txt", "text/plain");
    Hashtable_put(ext_to_mime, "gif", "image/gif");
    Hashtable_put(ext_to_mime, "png", "image/png");
    Hashtable_put(ext_to_mime, "ico", "image/x-icon");
}

/**
 * @brief destroys the file extension to MIME type hash table
 */
void destroy_mime_db()
{
    Hashtable_destroy(ext_to_mime);   // do before server terminates
}

/**
 * @brief Return a MIME type for a given filename
 * 
 * Details: Uses the ext_to_mime hash table since retrieval in a hash table is O(1)
 * 
 *          Previously done like this (O(n) time complexity):
 *          if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) { return "text/html"; }
 *          if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) { return "image/jpg"; }
 *          if (strcmp(ext, "css") == 0) { return "text/css"; }
 *          if (strcmp(ext, "js") == 0) { return "application/javascript"; }
 *          if (strcmp(ext, "json") == 0) { return "application/json"; }
 *          if (strcmp(ext, "txt") == 0) { return "text/plain"; }
 *          if (strcmp(ext, "gif") == 0) { return "image/gif"; }
 *          if (strcmp(ext, "png") == 0) { return "image/png"; }
 * 
 * @param[in] filename A string storing the name of a file
 * @return A string storing the MIME type of the file
 */
char *get_mime_type(char *filename)
{
    char *ext = strrchr(filename, '.');

    if (ext == NULL) {
        return DEFAULT_MIME_TYPE;
    }
    
    ext++;
    strlower(ext);

    printf("The File extension of '%s' is '%s'\n", filename, ext);
    
    create_mime_db();
    char *mime = Hashtable_get(ext_to_mime, ext);
    destroy_mime_db(); 

    if(mime == NULL)
        return DEFAULT_MIME_TYPE;

    return mime;
}