#include <stdint.h>
#define BRSH_VERSION "0.1"

#define BUFFER_SIZE 0x1000
#define COMMAND_SIZE 512 // defines the maximum size of a command or an argument
#define PATH_ELEMENTS 2

const static char* DELIMITERS[] = {"|", "&&"};
const static char* PATH[2] = {"/bin", "/usr/bin"};

enum delimiter_type {PIPE = 0, AND = 1, REDIRECT = 2};
typedef char delimiter;
typedef char *block;

typedef struct {
    char **argv;
    int argc;
} command_info;

void parse_buffer(char *buffer);
command_info parse_block(block buffer);
char *remove_whitespace(char *buffer);
char *parse_path(char *command);
char *parse_prompt(void);

void pipe_two(char *argv1[], char *argv2[], int argc1, int argc2);
void write_pipe(int filedes, char *input);
char *read_pipe(int filedes);

void error_kill(char *func, char *error);

int execute_builtin(char **argv);