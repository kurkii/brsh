#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
enum {
    BRSH_KEY_PATH_INDEX = 0,
    BRSH_KEY_PROMPT_INDEX,
    BRSH_KEY_PROMPTARGS_INDEX,
};

#define KEY_LENGTH 256
#define VALUE_LENGTH 256
#define VALUE_STRING_SIZE 256
#define VALUE_BOOL_SIZE 6

#define BRSH_LINE_MAX 2048

#define KEY_NUM 3

#define CONFIG_FILENAME ".brsh_config"

typedef struct {
    char *name; // the string literal which the key is referred to in the config
    uint8_t type; // type of value that the key accepts (int, string, float, bool)
    union {
        char string_value[VALUE_STRING_SIZE];
        int int_value;
        float float_value;
        bool bool_value;
    };
} brsh_key;

enum {
    KEY_TYPE_INT = 0,
    KEY_TYPE_STRING,
    KEY_TYPE_FLOAT,
    KEY_TYPE_BOOL,
};

int keyname_to_index(char *name);
int write_config(char *name, char *value);
int read_config(void);
void config_initial_setup();
void get_config_path();
brsh_key get_key(int index);
void default_config_setup(FILE *file);

#endif
