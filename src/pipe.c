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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "brsh.h"

// fd[0] == stdin, fd[1] == stdout

void pipe_two(char *argv1[], char *argv2[], int argc1, int argc2){
  int pipefd[2];
  
  if(pipe(pipefd)){
    error_kill("pipe_two()", "Failed to create pipe");
  }

  pid_t p1 = fork();

  if (p1 < 0){
    error_kill("pipe_two()", "p1: fork() failed");
  }else{

    if(p1 == 0){ // child
      close(pipefd[0]); // close read end of the pipe

      if(dup2(pipefd[1], 1) == -1){ // replace stdout with the write end of pipe
        char *error = strerror(errno);
        printf("brsh: pipe_two: dup2() at p1: %s", error);
      }

      char *executable = parse_path(argv1[0]); // finds executable in path

      if(executable == NULL){
        free(executable);
        exit(1);
      }

      pid_t p3 = fork();

      if(p3 < 0){
        error_kill("pipe_two()", "p3: fork() failed");
      }

      if(p3 == 0){
        if(execv(executable, argv1) == -1){
          char *error = strerror(errno);
          fprintf(stderr, "brsh: %s: %s\n", executable, error);
          free(executable);
          exit(1);
        }
      }

      wait(NULL);
      close(pipefd[1]);

      free(executable);

      exit(0);
    }else{       // parent
      //printf("am parent\n");

      close(pipefd[1]);

      //printf("forking second program:\n");
      wait(NULL);
      pid_t p2 = fork();
      if(p2 < 0){
        error_kill("pipe_two()", "p2: fork() failed");
      }else{
        if(p2 == 0){

          close(pipefd[1]); // close write end of pipe

          char *executable = parse_path(argv2[0]);

          if(executable == NULL){
            free(executable);
            exit(1);
          }

          if(dup2(pipefd[0], 0) == -1){ // replace stdin with read end of pipe
            char *error = strerror(errno);
            printf("brsh: pipe_two: dup2() at p2: %s", error);
            free(executable);
            exit(1);
          }

          pid_t p4 = fork();

          if(p4 < 0){
            error_kill("pipe_two()", "p4: fork() failed");
          }
          
          if(p4 == 0){
            if(execv(executable, argv2) == -1){
              char *error = strerror(errno);
              fprintf(stderr, "brsh: %s: %s\n", executable, error);
              free(executable);
              exit(1);
            }
          }
          wait(NULL);
          close(pipefd[0]);
          free(executable);
          exit(0);
        }
        wait(NULL); // parent
      }
    }
  }
  wait(NULL);
  return;
}

/*
  read_pipe() and write_pipe() to be removed, they serve no use.

*/

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