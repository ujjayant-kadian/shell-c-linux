#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
// Some extra commands
int cd(char **args);
int hello(char **args);
int shell_exit(char **args);

char *builtInStrings[] = { "cd", "hello", "exit" };
int (*builtInFuncs[]) (char **) = { &cd, &hello, &shell_exit };

int numberOfBuiltIns() {
  return (sizeof(builtInStrings) / sizeof(char *));
}
// Command Description
int cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "ujjsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("ujjsh");
    }
  }
  return 1;
}
int shell_exit(char **args)
{
  return 0;
}
int hello(char **args)
{
  printf("Welcome to the shell!!!\n");
  printf("My name is Ujjayant Kadian.\n 102003606\n");
  printf("Almost all linux commands work (except piping and redirection). Although I have made some changes in pwd.\n");
  return 1;
}
// For Launching the shell
int launch_ujjsh(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("ujjsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("ujjsh");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}
// For Executing Shell
int executeShell(char **args)
{
  int i;
  if (args[0] == NULL) {
    return 1;
  }
  for (i = 0; i < numberOfBuiltIns(); i++) {
    if (strcmp(args[0], builtInStrings[i]) == 0) {
      return (*builtInFuncs[i])(args);
    }
  }
  return launch_ujjsh(args);
}
// For reading the input from terminal
char *readLine(void)
{
#ifdef USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufferSize = 0;
  if (getline(&line, &bufferSize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);
    } else  {
      perror("ujjsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define BUFSIZE 1024
  int bufferSize = BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufferSize);
  int c;

  if (!buffer) {
    fprintf(stderr, "ujjsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
    // If we have exceeded the buffer, reallocate.
    if (position >= bufferSize) {
      bufferSize += BUFSIZE;
      buffer = realloc(buffer, bufferSize);
      if (!buffer) {
        fprintf(stderr, "ujjsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}
#define tokenBufSize 64
#define tokenDelim " \t\r\n\a"
// For spliiting the input into tokens
char **splitLine(char *line)
{
  int bufferSize = tokenBufSize, position = 0;
  char **tokens = malloc(bufferSize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "ujjsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, tokenDelim);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufferSize) {
      bufferSize += tokenBufSize;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufferSize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "ujjsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, tokenDelim);
  }
  tokens[position] = NULL;
  return tokens;
}

int main(int argc, char **argv)
{
  char *line;
  char **args;
  int status;
  do {
    printf("UjjShell :) >>> ");
    line = readLine();
    args = splitLine(line);
    status = executeShell(args);

    free(line);
    free(args);
  } while (status);
  return 0;
}