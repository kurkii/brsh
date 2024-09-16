#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#define BRSH_KEY_PATH_INDEX 0

#define KEY_LENGTH 256
#define VALUE_LENGTH 256
#define VALUE_STRING_SIZE 256
#define VALUE_BOOL_SIZE 6

#define BRSH_LINE_MAX 2048

#define KEY_NUM 1

#define CONFIG_FILENAME ".brsh_config"

typedef struct {
    char *name; // the string literal which the key is referred to in the config
    uint8_t type; // type of value that the key accepts (int, string, float, bool)
    union {
        char string_value[VALUE_STRING_SIZE];
        int int_value;
        float float_value;
        char bool_value[VALUE_BOOL_SIZE];
    };
} brsh_key;

enum {
    KEY_TYPE_INT = 0,
    KEY_TYPE_STRING = 1,
    KEY_TYPE_FLOAT = 2,
    KEY_TYPE_BOOL = 3
};

int keyname_to_index(char *name);
int write_config(char *name, char *value);
int read_config(void);
void config_initial_setup();
void get_config_path();
brsh_key get_key(int index);

#endif
