/*

    pipe.c - handling piping and redirections

    pipe(int fd[2]):
    fd[0] - read end
    fd[1] - write end

    # CONCEPT
    pipe_two() - forks two programs then pipes the stdout of first program to stdin of the second program:

      void pipe_two(char *argv1[], char *argv2[], int argc1, int argc2)

      eg:

      ls -l | less <- opens fd[0]. redirects data from pipe to stdin
        ^
      opens fd[1]. redirects stdout to pipe

      basic outline:
      - fork program1
      - fork program2
      - program1 writes in fd[1]
      - program2 reads fd[0]
      - we pass the data from fd[0] as program2's stdin
    
*/

enum{
  READ = 0,
  WRITE = 1,
};

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include "brsh.h"
#include "debug.h"


int *create_pipefd(int num){
  
  int *pipes = malloc(sizeof *pipes * num * 2); // allocate memory for the pipes
  if(pipes == NULL){
    perror("malloc() failed");
    exit(EXIT_FAILURE);
  }

  for(size_t i = 0; i < num; i++){
    if(pipe(pipes + i*2) == -1){ // create the pipes
      perror("pipe() failed");
      exit(EXIT_FAILURE);
    }
  }

  return pipes;
}

bool configure_pipes(int *pipefd, int command, int pipe_count){
    if(command == 0){
      // first command
      return dup2(pipefd[command*2 + WRITE], STDOUT_FILENO) != -1; // first command, so we only write to stdout
    }else if(command == pipe_count){
      // last command
      return dup2(pipefd[(command-1)*2 + READ], STDIN_FILENO) != -1; // last command, so we only read from stdin
    }else {
      return dup2(pipefd[command*2 + WRITE], STDOUT_FILENO) != -1 && // middle command, so we do both
      dup2(pipefd[(command-1) * 2 + READ], STDIN_FILENO) != -1;
    }
}

int brsh_pipe(command_info *commandz, int pipe_counts, int command_counts){
  /*  ignore all user input, simply run the exact same code as other
      program */
  int pipe_count = 2;
  int command_count = 3;
  /* allocate memory for commands */
  command_info *commands = malloc(3 * sizeof(command_info));
  commands[0].argv = malloc(4096 * sizeof *commands[0].argv);
  commands[1].argv = malloc(4096 * sizeof *commands[1].argv);
  commands[2].argv = malloc(4096 * sizeof *commands[2].argv);
  
  #define COMMAND_SIZE 512
  for(size_t x = 0; x < BUFFER_SIZE; x++){
      commands[0].argv[x] = malloc(COMMAND_SIZE*sizeof(char)); // malloc each element of the array seperately
      memset(commands[0].argv[x], 0, COMMAND_SIZE); // zero out the memory
      commands[1].argv[x] = malloc(COMMAND_SIZE*sizeof(char)); // malloc each element of the array seperately
      memset(commands[1].argv[x], 0, COMMAND_SIZE); // zero out the memory
      commands[2].argv[x] = malloc(COMMAND_SIZE*sizeof(char)); // malloc each element of the array seperately
      memset(commands[2].argv[x], 0, COMMAND_SIZE); // zero out the memory
  }
  /* set the commands */
  memcpy(commands[0].argv[0], "/bin/w", strlen("/bin/w"));
  commands[0].argv[1] = NULL;
  commands[0].argc = 1;

  memcpy(commands[1].argv[0], "/bin/grep", strlen("/bin/grep"));
  memcpy(commands[1].argv[1], "tty2", strlen("tty2"));
  commands[1].argv[2] = NULL;
  commands[1].argc = 2;

  memcpy(commands[2].argv[0], "/bin/grep", strlen("/bin/grep"));
  memcpy(commands[2].argv[1], "t", strlen("t"));
  commands[2].argv[2] = NULL;
  commands[2].argc = 2;

  printf("commands[1].argv[1]: %s\n", commands[0].argv[0]);

  printf("pipe_count 2: %d\n", pipe_count);
  printf("command_count 2: %d\n", command_count);

  /* debug prints*/
  for (int i = 0; i < command_count; i++) {
    for(int j = 0; j < commands[i].argc+1; j++){
      fprintf(stderr, "commands[%d].argv[%d]: %s\n", i, j, commands[i].argv[j]);
    }
  }

  /* create the required pipes*/
  int* pipes = create_pipefd(pipe_count);

  
  for(size_t i = 0; i < command_count; i++){
    pid_t child = fork();

    switch (child) {
      case -1:
        fprintf(stderr, "Failed to fork()");
        exit(EXIT_FAILURE);
      case 0:
        /* child */

        
        if(configure_pipes(pipes, i, pipe_count) == false){
          perror("configure_pipes failed\n");
          return 1;
        }

        /* close the pipes*/
        for (size_t j = 0; j < pipe_count; j++) {
          close(pipes[j*2+READ]);
          close(pipes[j*2+WRITE]);
        }

        free(pipes);

        if(execvp(commands[i].argv[0], commands[i].argv) != 0){
          perror("execvp() failed");
          exit(EXIT_FAILURE);
        }

        break;
      default:
        break;
    }

  }
  return 0;
}



char *read_pipe(int filedes){
  char *output = malloc(BUFFER_SIZE * 512); // <- remember to free()!!

  char c;
  size_t i = 0;
  FILE *stream = fdopen(filedes, "r");


  if(stream == NULL){
    char *error = strerror(errno);
    error_kill("write_pipe()", error);
  }


  while ((c = fgetc(stream)) != EOF) { // parse the stream byte for byte and put it in the `output` buffer
    
    output[i] = c;
    i++;
  }

  return output;
  
}

void write_pipe(int filedes, char *input){
  FILE *stream = fdopen(filedes, "w");

  if(stream == NULL){
    char *error = strerror(errno);
    error_kill("write_pipe()", error);
  }
  
  for(size_t i = 0; i < strlen(input)+1; i++){
    fputc(input[i], stream);
  }

  return;
}
  //printf("pipe_count: %d\n", pipe_count);


/* Writes an EOF to the specified file descriptor*/
void write_EOF(int filedes){
  FILE *stream = fdopen(filedes, "w");

  if(stream == NULL){
    error_kill("write_EOF()", strerror(errno));
  }

  fputc(EOF, stream);
  fclose(stream);
  return;
}