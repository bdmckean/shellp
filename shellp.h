#ifndef SHELLP_H 
#define SHELLP_H

#include <stdbool.h>

extern int exec_cmds(char ** cmd_list, int num_args, bool batch);
extern char ** parseline(char * line, int * num_args);
extern int debug;

#endif
