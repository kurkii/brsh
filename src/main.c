/*
    # CONCEPT

    char* of delimiters "| < > && " etc...
    chop up input into blocks based on delimiters
    set delimiters in order
    then chop up the blocks into the command and its arguments using spaces as delimiter
    execute first command based on first delimiter (pipes and such)
    continue until reach end of commands

*/

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "brsh.h"
#include "config.h"
#include "debug.h"
char buffer[BUFFER_SIZE];

block block_array[BUFFER_SIZE] = {NULL};
uint16_t block_count = 0;
delimiter delimiter_array[BUFFER_SIZE] = {0};
uint16_t delimiter_count = 0;

int main(){
    
    //signal(SIGINT, sigint_handler);

    config_initial_setup();
    get_config_path();
    if(read_config() != 0){
        printf("Failed to read config, bailing.\n");
        exit(1);
    }

    char* prompt = parse_prompt();

    for(;;){

        printf("%s", prompt);

        char *ret = fgets(buffer, BUFFER_SIZE, stdin);

        if(ret == NULL){ // (ret == null) means EOF. can appear from eg. a CTRL-D
            printf("\nEOF, exiting...\n");
            exit(0);
        }

        if(buffer[0] == 10){ // if input is empty
            continue;
        }

        parse_buffer(buffer);
        
        if(delimiter_count == 0){ // if there is no piping/redirections happening, simply execute the command
            command_info command = parse_block(block_array[0]);
            
            if(execute_builtin(command.argv) == 0){ // first check if there is a built in command
                for(size_t x = 0; x < BUFFER_SIZE; x++){
                    free(command.argv[x]); // free each element of the argv array
                }

                free(command.argv);
                continue;
            }; 

            char *executable = parse_path(command.argv[0]);

            if(executable == NULL){
                for(size_t x = 0; x < BUFFER_SIZE; x++){
                    free(command.argv[x]); // free each element of the argv array
                }

                free(command.argv);
                continue;
            }
            
            pid_t p = fork();
            if(p < 0){
                error_kill("main()", "fork() failed");
            }
            if(p == 0){
                if(execv(executable, command.argv) == -1){
                    int error = errno;
                    printf("brsh: %s: %s\n", command.argv[0], strerror(error));
                    exit(1);
                }
            }
 
            wait(NULL);
            
            free(executable);

            for(size_t x = 0; x < BUFFER_SIZE; x++){
                free(command.argv[x]); // free each element of the argv array
            }

            

            free(command.argv);

            goto cleanup;
        }else{

            command_info *commands = malloc(sizeof(command_info) * block_count);

            for(size_t i = 0; i < block_count; i++){
                commands[i] = parse_block(block_array[i]);
            }
            
            brsh_pipe(commands, delimiter_count, block_count);

            /* wait for children to finish */
            for(int i = 0; i < block_count; i++){
                wait(NULL);
            }

            /* free everything, maybe find a better solution to memory? */
            for(size_t i = 0; i < block_count; i++){
                for(size_t x = 0; x < BUFFER_SIZE; x++){
                    free(commands[i].argv[x]);
                }
                free(commands[i].argv);

            }

            free(commands);

        }
        cleanup:

        *block_array = NULL;
        block_count = 0;
        strcpy(delimiter_array, "");
        delimiter_count = 0;
    }

    free(prompt);
}

