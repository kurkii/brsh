/*

    pipe.c - handling piping and redirections

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

int brsh_pipe(command_info *commands, int pipe_count, int command_count){

  /* create the required pipes*/
  int* pipes = create_pipefd(pipe_count);

  for(int i = 0; i < command_count; i++){
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
        for (int j = 0; j < pipe_count; j++) {
          close(pipes[j*2+READ]);
          close(pipes[j*2+WRITE]);
        }

        char *executable = parse_path(commands[i].argv[0]);

        if(executable == NULL){
          exit(EXIT_FAILURE);
        }

        if(execvp(executable, commands[i].argv) != 0){
          perror("execvp() failed");
          exit(EXIT_FAILURE);
        }

        break;
      default:
        break;
    }
  }

  for (int j = 0; j < pipe_count; j++) {
    close(pipes[j*2+READ]);
    close(pipes[j*2+WRITE]);
  }

  free(pipes);
  return 0;
}

/* char *read_pipe(int filedes){
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

void write_EOF(int filedes){
  FILE *stream = fdopen(filedes, "w");

  if(stream == NULL){
    error_kill("write_EOF()", strerror(errno));
  }

  fputc(EOF, stream);
  fclose(stream);
  return;
} */