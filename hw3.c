#include "hw3.h"

/* This function prints a helpful message for
   those that want or need more information
   about the shell. */
void help(){
  printf("Author: Carlos Antonio McNulty\n"
       "CS361, Fall 2018\n"
       "Homework 3\n"
       "This is a simple shell program written\n"
       "for CS361. It supports two built-in commands:\n"
       "[help] prints this message\n"
       "[exit] exits the shell.\n");
}


/* This function is the signal handler for
   the SIGINT and SIGTSTP signals. */
void handler(int sig){
  /* Process SIGINT */
  if(sig == SIGINT){
    write(1, "\ncaught sigint\n", 15); 
  }/* Process SIGTSTP */
  else if(sig == SIGTSTP){
    write(1, "\ncaught sigtstp\n", 16); 
  }
  /* Redisplay prompt */
  write(1, "CS361> ", 7); 
}


/* This function parses the redirection files from the
   buffer. It replaces the first redirection symbol with
   a null-terminator. */
int parse_files(char *buffer, char *infile, char *outfile){
  char *delim, symbol;
  /* Test if line contains redirection symbols */
  if(strpbrk(buffer, "<>")){
    /* Change terminator to space */
    buffer[strlen(buffer)] = ' ';
    /* Parse until there are no more redirection symbols */
    while((delim = strpbrk(buffer, "<>"))){
      /* Save symbol */
      symbol = *delim;
      /* Replace delimiter with terminator */
      *delim = '\0';
      buffer = delim + 1;
      /* Skip leading white space */
      while(*buffer && isspace(*buffer)){
        buffer++;
      }
      /* Parse next token from buffer */
      if((delim = strpbrk(buffer, "\t "))){
        *delim = '\0';
        /* Copy to corresponding buffer
           based on symbol */
        if(symbol == '<'){
          strcpy(infile, buffer);
        }
        else{
          strcpy(outfile, buffer);
        }
      }
      /* Update buffer to just past last token */
      buffer = delim + 1;
    }
  }
}


/* This function parses the command line arguments
   from the buffer. It returns the number of command
   line arguments. */
int parse_args(char *buffer, char** argv){
  char *delim;
  int argc = 0;
  /* Change terminator to space */
  buffer[strlen(buffer)] = ' ';
  /* Skip leading white space */
  while(*buffer && isspace(*buffer)){
    buffer++;
  }
  /* Parse next token from buffer */
  while((delim = strpbrk(buffer, "\t "))){
    *delim = '\0';
    /* Enter token into argument vector */
    argv[argc++] = buffer;
    /* Update buffer to just past last token */
    buffer = delim + 1;
    /* Skip leading whitespace */
    while(*buffer && isspace(*buffer)){
      buffer++;
    }
  }
  /* Terminate argument vector */
  argv[argc] = NULL;
  /* Return argument count */
  return argc;

}


/* This function checks if the given command is
   a built-in command. If it is, then it processes
   that command and returns true. Otherwise it
   returns false.
 */
int process_builtin_command(char *command){
  // Process exit command
  if(strcmp(command, "exit") == 0){
    exit(EXIT_SUCCESS);
  }// Process help command
  else if(strcmp(command, "help") == 0){
    help();
    return TRUE;
  }
  return FALSE;
}


/* This function parses the command line arguments
   and redirection files from the command passed in.
   It then forks and attempts to execute the command.
   It returns the process id of the newly created process.
   It returns -1 if the command was a built-in command and
   no new process was created. */
int process_command(char *command){
  /* Create buffers for files and the argument vector */
  char infile[MAX_ARG], outfile[MAX_ARG];
  char *argv[MAX_ARGS]; 
  /* Set file buffers to empty set */
  infile[0] = '\0'; outfile[0] = '\0';
  /* Parse redirection file, removes section from line */
  parse_files(command, infile, outfile);
  /* Parse arguments */
  int argc = parse_args(command, argv);
  /* Process built-in command */
  if(argv[0] && process_builtin_command(argv[0])){
    return -1;
  }
  // Flush standard output before forking
  fflush(stdout);
  // Create new process 
  int pid;
  if((pid = fork()) == 0){
    /* Check for redirection */
    if(strlen(infile) > 0){
      /* Open input file */
      int desc = open(infile, O_RDONLY);
      /* Test if file was opened */
      if(desc < 0){
        fprintf(stderr, "-bash: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      /* Redirect stdin */
      dup2(desc, STDIN_FILENO);
    }
    if(strlen(outfile) > 0){
      /* Open output file */
      int desc = open(outfile, O_WRONLY | O_CREAT);
      /* Redirect stdout */
      dup2(desc, STDOUT_FILENO);
    }
    /* Execute command */
    if(execvp(argv[0], argv) < 0){
      fprintf(stderr, "-bash: %s: %s\n", argv[0], strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
  return pid;
}


/* This function parses the commands from the user input.
   It then parses and executes each individual command. */
void process_commands(char *buffer){
  /* Create buffer for command */
  char command[MAX_ARG];
  /* Parse commands using ";" as a delimiter */
  buffer = strtok(buffer, ";");
  while(buffer != NULL){
    /* Copy buffer to command */  
    strcpy(command, buffer);
    /* Process command */
    int pid = process_command(command);
    if(pid >= 0){
      int status;
      /* Wait for command to terminate */
      waitpid(pid, &status, 0);
      printf("pid:%d status:%d\n", pid, WEXITSTATUS(status));
    }
    /* Get next token */
    buffer = strtok(NULL, ";");
  }
}

int main(){
  /* Create input buffers */
  char buffer[MAX_LINE];  
  /* Install SIGINT handler */
  signal(SIGINT, handler);
  /* Install SIGTSTP handler */
  signal(SIGTSTP, handler);
  /* Process user input */
  while(TRUE){
    printf("CS361 > ");
    fgets(buffer, MAX_LINE, stdin);
    /* Strip trailing newline/carriage return */
    buffer[strcspn(buffer, "\n\r")] = '\0';
    /* Process user commands */
    process_commands(buffer);     
  }
  return EXIT_SUCCESS;
}

