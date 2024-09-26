/*

    builtin.c - provides built-in functions included in the shell

    OUTLINE
        - array of function pointers
        - in the main file for loop check if argument is equal to a built in command

*/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define BUILTIN_NUM 1
static char *builtin_list[BUILTIN_NUM] = {"exit"};

void builtin_exit(char **argv){
    exit(0);
}

void (*func_ptr[BUILTIN_NUM])(char**) = {builtin_exit};

int execute_builtin(char **argv){

    if(argv == NULL){
        return -1;
    }

    _Bool was_found = 0;
    for(size_t i = 0; i < BUILTIN_NUM; i++){
        if(strcmp(argv[0], builtin_list[i]) == 0){
            was_found = 1;
            (*func_ptr[i])(argv);
        }
    }

    if(was_found == 1){
        return 0;
    }

    return -1;

}