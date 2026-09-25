#ifndef DOTENV_H
#define DOTENV_H

#include <stdio.h>

// Struct to store an individual key-value configuration pair
typedef struct {
    char *key;
    char *value;
} ConfigItem;

// Public interface declarations
int load_dotenv(const char *filename);
const char* get_env(const char *key);
const char* get_env_or(const char *key,const char *default_value);
void free_dotenv(void);

#endif // DOTENV_H
