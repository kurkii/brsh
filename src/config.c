/*

    config.c - reading the brsh config, writing to it.


    brsh_key keys[KEY_NUM] - holds all the keys that are found in config. its 


    CONFIG FORMAT:
    key=value

    OUTLINE:
        read_config():
            - open config file
            - read line_num by line_num
            - delimiter is =, put first string in char *key, and second string in char* value
            - check if key is valid (hashmap?)

    EXPLANATIONS:
    `name` is the string which the key appears as in the config. for example the path key appears as `path` in the config
 

*/
#include <limits.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "config.h"

brsh_key keys[KEY_NUM] = {0};

char config_path[PATH_MAX] = {0}; // Path to the config file

void set_key(int index, char *name, int type){
    keys[index].name = name;
    keys[index].type = type;
    switch (type) {
        case KEY_TYPE_BOOL:
            memset(keys[index].bool_value, 0, VALUE_BOOL_SIZE);
            break;
        case KEY_TYPE_STRING:
            memset(keys[index].string_value, 0, VALUE_STRING_SIZE);
            break;
        case KEY_TYPE_INT:
            keys[index].int_value = -1;
            break;
        case KEY_TYPE_FLOAT:
            keys[index].float_value = -1;
            break;
        default:
            printf("CONFIG: setkey(): %d is not a valid type!\n", type);
            exit(1);

    }
}

/*
    void config_initial_setup()

    setups up the constant keys. when adding a key, remember to increment KEY_NUM
*/

void config_initial_setup(){
    set_key(BRSH_KEY_PATH_INDEX, "path", KEY_TYPE_STRING);
}

void get_config_path(){
    struct passwd *pw = getpwuid(getuid());
    memcpy(config_path, pw->pw_dir, strlen(pw->pw_dir));
    strcat(config_path, "/");
    strcat(config_path, CONFIG_FILENAME);
    return;
} 
/*

    read_config()

    - reads the config file and parses its contents. the values of the config are stored in the `keys` array


*/
int read_config(){
    FILE *fileptr;

    char line[BRSH_LINE_MAX];

    int line_num = 0; // line counter

    fileptr = fopen(config_path, "r");

    if(fileptr == NULL){
        fileptr = fopen(config_path, "w"); // if the config file doesn't exist, then create it

        if(fileptr == NULL){
            int error = errno;
            printf("brsh: %s: %s: %s\n", "read_config()", "fopen()", strerror(error));
            return -1;
        }
    }
    char *key = malloc(KEY_LENGTH*sizeof(char));

    char *value = malloc(VALUE_LENGTH*sizeof(char));
    while(fgets(line, BRSH_LINE_MAX, fileptr) != NULL){
        
        line_num++;
        memset(key, 0, KEY_LENGTH);
        memset(value, 0, VALUE_LENGTH);

        unsigned long full_str_length = strlen(line);

        for(size_t i = 0; i < full_str_length-1; i++){
            if(line[i] == '='){
                line[i] = '\0'; // set the NUL character of the key string
                i++;

                unsigned long key_str_length = strlen(line); // strlen of the key

                if(key_str_length > KEY_LENGTH){
                    printf("CONFIG ERROR: key length at line %d is over the maximum (%d)", line_num, KEY_LENGTH);
                    fclose(fileptr);
                    free(key);
                    free(value);
                    return -1;
                }

                memcpy(key, line, key_str_length); // copies the line to the point of the newly set NUL character

                unsigned long value_str_length = full_str_length - key_str_length; // the length of the value is the length of the line minus the length of the key

                if(value_str_length > VALUE_LENGTH){
                    printf("CONFIG ERROR: value length at line %d is over the maximum (%d)", line_num, VALUE_LENGTH);
                    fclose(fileptr);
                    free(key);
                    free(value);
                    return -1;
                }

                memcpy(value, line+i*sizeof(char), value_str_length); // offsets line and copies whatever is left of the line into `value`

                break;
            }
        }

        printf("key: %s\n", key);

        int index = keyname_to_index(key);

        if(index == -1){
            printf("CONFIG ERROR on line %d: key `%s` not a valid key\n", line_num, key);
            fclose(fileptr);
            free(key);
            free(value);
            return 1;
        }
        value[strcspn(value, "\n")] = 0; // remove trailing newline from `value`
        printf("value: %s\n", value);

        switch (keys[index].type) { // depending on the type of the key, we must use different data types
            int valuetoint;
            case KEY_TYPE_INT: 
                valuetoint = strtol(value, NULL, 10);

                if(valuetoint == 0){
                    printf("CONFIG ERROR: `%s` is not a valid integer!\n", value);
                    free(value);
                    free(key);
                    fclose(fileptr);
                    return 1;
                }

                keys[index].int_value = valuetoint;
                break;
            case KEY_TYPE_STRING:

                if(strlen(value) > VALUE_STRING_SIZE){
                    printf("The value of key `%s` is too big! (over %d characters)\n", key, VALUE_STRING_SIZE);
                    return 1;
                }

                strcpy(keys[index].string_value, value);
                break;
            case KEY_TYPE_BOOL:

                if(strlen(value) > VALUE_STRING_SIZE){
                    printf("The value of key `%s` is too big! (over %d characters)\n", key, VALUE_BOOL_SIZE);
                    return 1;
                }

                strcpy(keys[index].bool_value, value);
                break;
            default:
                printf("CONFIG ERROR: invalid type key type`%d`\n", keys[index].type);
                fclose(fileptr);
                exit(1);
        }

    }

    fclose(fileptr);
    free(key);
    free(value);
    return 0;
}


/* writes a key-value pair to the config file*/
int write_config(char* name, char* value){
    FILE *fileptr = fopen(config_path, "w");

    if(fileptr == NULL){
        int error = errno;
        printf("brsh: %s: %s: %s\n", "write_config()", "fopen()", strerror(error));
        return -1;
    }

    fprintf(fileptr, "%s=%s\n", name, value);
    return 0;
}

/*
    int keyname_to_index(char *name)

    finds the keys[] index with the corresponding name

    returns:
        index on success
        -1 on failure
*/

int keyname_to_index(char *name){
    if(name == NULL){
        return -1;
    }

    for(size_t i = 0; i < KEY_NUM; i++){
        if(strcmp(name, keys[i].name) == 0){
            return i;
        }
    }

    return -1;
}

/*
    returns key[index]
*/
brsh_key get_key(int index){

    if(index > KEY_NUM){
        return (brsh_key){0};
    }

    return keys[index];
}


/* int main(){
    config_initial_setup();
    get_config_path();
    if(read_config() != 0){
        printf("Failed to read config, bailing.\n");
        exit(1);
    }

    printf("cool, lets acces this shit\n");

    int index = keyname_to_index("path");
    if(index == -1){
        printf("failed :(\n");
    }

    printf("lets see what we got: %s\n", keys[index].string_value);

    //free(keys[index].string_value);

    index = keyname_to_index("duck");
    if(index == -1){
        printf("failed :(\n");
    }
    printf("duck: %s\n", keys[index].string_value);

} */