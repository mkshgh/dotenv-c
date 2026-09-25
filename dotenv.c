#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dotenv.h"

#ifdef _WIN32

static int getline(
    char **lineptr,
    size_t *n,
    FILE *stream)
{
    if (!lineptr || !n || !stream)
        return -1;

    if (*lineptr == NULL || *n == 0) {
        *n = 256;
        *lineptr = malloc(*n);

        if (!*lineptr)
            return -1;
    }

    size_t pos = 0;

    int c;

    while ((c = fgetc(stream)) != EOF) {

        if (pos + 1 >= *n) {

            size_t new_size = *n * 2;

            char *new_ptr =
                realloc(*lineptr, new_size);

            if (!new_ptr)
                return -1;

            *lineptr = new_ptr;
            *n = new_size;
        }

        (*lineptr)[pos++] = (char)c;

        if (c == '\n')
            break;
    }

    if (pos == 0 && c == EOF)
        return -1;

    (*lineptr)[pos] = '\0';

    return (int)pos;
}

#endif

static ConfigItem *config_table = NULL;
static size_t config_count = 0;
static size_t config_capacity = 0;
static char *trim(char *str)
{
    char *end;

    while (isspace((unsigned char)*str))
        str++;

    if (*str == '\0')
        return str;

    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
        *end-- = '\0';

    return str;
}
// Internal function to add an item to our dynamic table
static void add_to_table(const char *key, const char *value)
{
    if (config_count >= config_capacity) {
        size_t new_capacity =
            config_capacity == 0 ? 4 : config_capacity * 2;

        ConfigItem *tmp = realloc(
            config_table,
            new_capacity * sizeof(ConfigItem)
        );

        if (tmp == NULL) {
            perror("Memory allocation failed for config table");
            exit(EXIT_FAILURE);
        }

        config_table = tmp;
        config_capacity = new_capacity;
    }

    char *key_copy = strdup(key);
    char *value_copy = strdup(value);

    if (key_copy == NULL || value_copy == NULL) {
        free(key_copy);
        free(value_copy);
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    config_table[config_count].key = key_copy;
    config_table[config_count].value = value_copy;

    config_count++;
}

// Search Function: Returns value by key name, or NULL if not found
const char *get_env(const char *key)
{
    if (key == NULL) {
        return NULL;
    }

    for (size_t i = config_count; i > 0; i--) {
        if (strcmp(config_table[i - 1].key, key) == 0) {
            return config_table[i - 1].value;
        }
    }

    return NULL;
}

const char *get_env_or(const char *key, const char *default_value)
{
    const char *value = get_env(key);
    return value ? value : default_value;
}

static int split_text(char *buffer)
{
    // Remove newline characters from the end of the line
    buffer[strcspn(buffer, "\r\n")] = '\0';

    // Trim left and right spaces from the complete line
    buffer = trim(buffer);

    // Ignore empty lines and comments
    if (buffer[0] == '#' || buffer[0] == '\0') {
        return 1;
    }

    // Split the line into key and value using '='
    char *key_token = strtok(buffer, "=");
    if (key_token == NULL)
        return 0;

    // Remove spaces around the key
    key_token = trim(key_token);

    // Get everything after '=' as the value
    char *value_token = strtok(NULL, "");

    // If no value exists, store an empty string
    if (value_token == NULL) {
        add_to_table(key_token, "");
        return 1;
    }

    // Remove spaces outside the value
    value_token = trim(value_token);

    // Assign empty placeholder if value is blank
    if (value_token[0] == '\0') {
        add_to_table(key_token, "");
        return 1;
    }

    // Get value length for quote checking
    size_t value_len = strlen(value_token);

    // Remove surrounding double quotes from the value
    if (value_token[0] == '"') {
        if (value_len >= 2 && value_token[value_len - 1] == '"') {
            value_token++;
            value_token[value_len - 2] = '\0';
        } else {
            printf(
                "Error: Missing closing quotation mark for key '%s'\n",
                key_token
            );
            return 0;
        }
    }

    // Save completely isolated copies to the permanent lookup registry
    add_to_table(key_token, value_token);

    return 1;
}

int load_dotenv(const char *filename){

    FILE *file = filename==NULL?fopen(".env","r"):fopen(filename,"r");

    if (file == NULL) {
        perror("Error opening ENV file");
        return EXIT_FAILURE;
    }

    char *buffer = NULL;
    size_t buffer_size = 0;

    while (getline(&buffer, &buffer_size, file) != -1) {
    if (split_text(buffer) == 0) {
            free(buffer);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    free(buffer);
    fclose(file);
    return EXIT_SUCCESS;
}

// Mandatory cleanup routine to free all dynamic memory chunks
void free_dotenv(void)
{
    for (size_t i = 0; i < config_count; i++) {
        free(config_table[i].key);
        free(config_table[i].value);
    }

    free(config_table);

    config_table = NULL;
    config_count = 0;
    config_capacity = 0;
}
