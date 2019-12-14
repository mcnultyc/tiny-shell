#ifndef HW3_H
#define HW3_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LINE 500
#define MAX_ARGS 20
#define MAX_ARG 100

#define FALSE 0
#define TRUE 1


int parse_files(char *buffer, char *infile, char *outfile);
int parse_args(char *buffer, char** argv);
int process_command(char *command);
void process_commands(char *buffer);
void handler(int sig);
int process_builtin_command(char *command);
void help();


#endif


