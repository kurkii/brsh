#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
/* error_kill()

    - called when the program has an error and needs to terminate
    - eg: error_kill("main()", "the cake is not here")

*/
void error_kill(char *func, char *error){ 
    fprintf(stderr, "brsh: %s: error: %s\n", func, error);
    exit(EXIT_FAILURE);
}

void error_kill_parent(char *func, char *error, pid_t parent){
    fprintf(stderr, "brsh: %s: error: %s\n", func, error);
    if(kill(parent, SIGTERM) == -1){
        fprintf(stderr, "Failed to kill parent\n");
    }
    exit(EXIT_FAILURE);
}