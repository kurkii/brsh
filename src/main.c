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
char buffer[BUFFER_SIZE];

block block_array[BUFFER_SIZE] = {NULL};
uint16_t block_count = 0;
delimiter delimiter_array[BUFFER_SIZE] = {0};
uint16_t delimiter_count = 0;

int main(){
    
    char* prompt = parse_prompt();

    for(;;){

        printf("%s", prompt);
          
        fgets(buffer, BUFFER_SIZE, stdin);

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
        }else{ // pipe the two commands (eventually replace this with a dynamic thing supporting multiple pipes)
            command_info command1;
            command_info command2;

            command1 = parse_block(block_array[0]);
            command2 = parse_block(block_array[1]);

            if(command1.argv == NULL || command2.argv == NULL){
                error_kill("main()", "argv == NULL");
            }

            pipe_two(command1.argv, command2.argv, command1.argc, command2.argc);

            for(size_t x = 0; x < BUFFER_SIZE; x++){
                free(command1.argv[x]);
                free(command2.argv[x]);
            }

            free(command1.argv);
            free(command2.argv);

            command1.argc = 0;
            command2.argc = 0;

        }
        cleanup:

        *block_array = NULL;
        block_count = 0;
        strcpy(delimiter_array, "");
        delimiter_count = 0;
        break;
    }

    free(prompt);
}

