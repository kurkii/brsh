/*

    parser.c - parsing input into usable executable commands

    CONCEPT
    - get buffer from main()
    - using delimiters put every block into a char* (block) array
    - put all the delimiters in order into the delimiter array
    - parse the first block and second block for commands, execute the first one and pipe/redirect into the second one or whatever the delimiter wants
    - continue until run out of delimiters

    two functions:
        parse_buffer() - gets buffer and goes through it and strtoks it into blocks and makes an array of delimiter types
        parse_block() - parse the blocks to get the commands and its arguments which is then used to execute the command

*/

#include <ctype.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include "config.h"
#include "brsh.h"

extern block block_array[BUFFER_SIZE]; // global block array from main.c file, used for storing blocks. the order of the elements [0-4096] is the order in which they appear in the buffer
extern uint16_t block_count; // global counter of the amount of blocks stored in the block array

extern delimiter delimiter_array[BUFFER_SIZE]; // global delimiter array from main.c file, used for storing delimiters. the order of the elements [0-4096] is the order in which they appear in the buffer
extern uint16_t delimiter_count; // global counter of the amount of delimiters stored in the delimiter array

/*

    so if you have block_array[0] then the delimiter which precedes it is delimiter_array[0],
    and if delimiter_array[0] is PIPE, then you pipe block_array[0] into block_array[1]

*/


void parse_buffer(char *buffer){
    uint64_t k = 0; // char counter
    uint64_t i = 0; // iteration counter

    if(buffer == NULL){
        return;
    }

    for(;;){

        if(buffer[i] == EOF){ // once we reach the actual EOF, break.
            break;
        }

        char *string = &buffer[k];
        switch (buffer[i]) {
            case '|':
                buffer[i] = '\0';
                i++; // go to the next character as this one is null terminated
                
                string = remove_whitespace(string);

                delimiter_array[delimiter_count] = PIPE;
                block_array[block_count] = string;
                delimiter_count++;
                block_count++;

                k = i; // equalize k, since the next block will start at that character (current i)
                break;
            case '&':
                buffer[i] = '\0';
                i++; // go to the next character as this one is null terminated
            
                string = remove_whitespace(string); // remove the extra whitespaces before the input

                delimiter_array[delimiter_count] = AND;
                block_array[block_count] = string;
                delimiter_count++;
                block_count++;

                k = i;
                break;
            case '\0': // end of string
                string = remove_whitespace(string); // <- issue

                block_array[block_count] = string;
                block_count++;

                return;

            default:
                break;
            }
        i++;
    }
    return;
}

/*
    char** parse_block(block buffer)
    - go through block
    - space is delimiter
    - store strings in char *string_array[]
    - return `string_array`

*/

command_info parse_block(block buffer){

    if(buffer == NULL){
        error_kill("parse_block()", "buffer == NULL");
    }

    char **string_array = malloc(BUFFER_SIZE * sizeof(char*)); // char* array

    for(size_t x = 0; x < BUFFER_SIZE; x++){
        string_array[x] = malloc(COMMAND_SIZE*sizeof(char)); // malloc each element of the array seperately
        memset(string_array[x], 0, COMMAND_SIZE); // zero out the memory
    }

    command_info command = {NULL};
    uint i = 0; // token counter
    uint j = 0; // character counter for string_array
    uint k = 0; // intermediary counter for buffer
    while (k < BUFFER_SIZE) {
        
        if(buffer[k] == '\0'){
            i++;
            break;
        }

        //printf("buffer[k+1]: %d\n", buffer[k+1]);
        if(buffer[k] == ' '){
            if(isspace(buffer[k+1]) != 0){
                k++;
                continue;
                
            }
            i++;
            k++;
            j = 0;
            continue;
        }

        string_array[i][j] = buffer[k];
        k++;
        j++;
    }

    free(string_array[i]); // free it since the assignment to NULL will prevent it from being freed
    string_array[i] = NULL; // last argument of argv[] is NULL

    command.argv = string_array; // pointer to strings is equal to an argv
    command.argc = i; // the number of commands is equal to an argc

    return command;
}

char *remove_whitespace(char *buffer){ 
    char *string = buffer;
    uint i = 0;
    while (string[i] == ' '){ // remove leading white space
        for(uint j = 0; j < strlen(string); j++){ // shift the string
            string[j] = string[j+1];
        }
    }
    
    for(int j = strlen(string+1); j > 0; j--){ // remove trailing white space
        if(!isspace(string[j])){
            break;
        }
        string[j] = '\0';
    }

    return string;
}

/* 

    int parse_path(char *command)

    - go through PATH
    - concatenate argv[0] with each PATH element
    - use some file function to check if file exists
    - if it does return concatenated string
    - else return strerror

 */

char *parse_path(char *command){
    bool file_found = 0;
    int error;
    size_t x = 0; // ndex

    brsh_key path_key = get_key(BRSH_KEY_PATH_INDEX);
    char *path = path_key.string_value;

    char *concat = malloc(BUFFER_SIZE * sizeof(char)); // <- remember to free

    if(fopen(command, "r") != NULL){ // if the command itself points to a file
        memcpy(concat, command, strlen(command)+1); // the reason we return concat instead of directly returning command is because of a free() in the callee, which would cause a double free error
        return concat;
    }

    for(size_t i = 0; i < PATH_ELEMENTS; i++){

        memset(concat, 0, BUFFER_SIZE*sizeof(char));
        for(; x < PATH_MAX; x++){
            if(path[x] == ':'){
                path[x] = '\0';
                x++;
                break;
            }

            concat[x] = path[x];

        }
        
        strcat(concat, "/"); // add a slash
        strcat(concat, command); // then concatenate command

        FILE *fileptr = fopen(concat, "r");
        error = errno;
        if(fileptr != NULL){
            file_found = 1;
            fclose(fileptr);
            break;
        }
    }
    

    if(file_found == 1){
        return concat;
    }
    //printf("du?\n");
    fprintf(stderr, "brsh: %s: %s\n", command, strerror(error));
    free(concat);
    return NULL;

}


/*
    parse_prompt()

    - primitive prompt creator with the layout `user@hostname: `, will probably be removed/refactored
    with config settings

*/
char *parse_prompt(void){
    #define PROMPT_DELIMITER "@"
    char *username = getlogin();

    if(username == NULL){
        error_kill("parse_prompt()", "username == NULL");
    }

    char hostname[_POSIX_HOST_NAME_MAX];
    if(gethostname(hostname, _POSIX_HOST_NAME_MAX) == -1){
        error_kill("parse_prompt()", "gethostname() unsuccessful");
    }

    if((strlen(username) + strlen(hostname)) > BUFFER_SIZE){
        error_kill("parse_prompt()", "buffer too small - blame me for being lazy");
    }

    char *concat = malloc(BUFFER_SIZE * sizeof(char)); // FREEEEEEE
    strcpy(concat, username);
    strcat(concat, PROMPT_DELIMITER);
    strcat(concat, hostname);
    strcat(concat, ": ");

    return concat;
}