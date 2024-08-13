#include <stdio.h>
#include <stdlib.h>
/* error_kill()

    - called when the program has an error and needs to terminate
    - eg: error_kill("main()", "the cake is not here")

*/
void error_kill(char *func, char *error){ 
    printf("brsh: %s: error: %s\n", func, error);
    exit(EXIT_FAILURE);
}