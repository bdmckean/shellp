#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "commands.h"
#include "shellp.h"

#define SIZE_DIR_STACK 10
char dir_stack[SIZE_DIR_STACK][1024];
int this_dir = 0;
int next_dir = 0;
int num_dirs = 0;


int unimplemented_command(char ** args){
    printf("Command not yet implemenetd\n" );
    return -1;
}

int quit(char ** args){
    // FIXME add wait until all processes are done
    exit(0);
}
struct cmd_fcn{
    char cmd[256];
    int (* fcn )(char ** args);
};


int init_dir_stack(void){
    char cwd[1024];
    char * cwdp = cwd;
    // Initialize directory stack
    if (getcwd(cwdp, sizeof(cwd)) != NULL){
        strcpy(dir_stack[0],cwdp);
        num_dirs = 1;
        next_dir = 1;
    }
    return 0;
}

int chg_dir(char ** args){
    if (args[1] == NULL || args[2] != NULL){
        fprintf(stderr, "shellp expected one argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0){
            perror("shellp");
        }
    }
    char cwd[1024];
    char * cwdp = cwd;

    if (getcwd(cwdp, sizeof(cwd)) != NULL){
        strcpy(dir_stack[next_dir],cwdp);
        this_dir = next_dir;
        if (num_dirs < SIZE_DIR_STACK) {
            next_dir++;
            num_dirs++;
        } else {
            if (next_dir == SIZE_DIR_STACK - 1){
                next_dir = 0;
            } else {
                next_dir++;
            }
        }
    }
    return 0; 
}

struct cmd_fcn cmds[] =  
    {
        {"set", unimplemented_command},
        {"cd", unimplemented_command},
        {"quit", quit},
        {"exit", quit},
        {"dx", unimplemented_command},
        {"dh",unimplemented_command},
        {"dc",unimplemented_command},
        {"fc", unimplemented_command}
    };



int (* (command_handler(char * cmd)))(char ** args){
    int num_cmds = sizeof(cmds)/sizeof(struct cmd_fcn);

    
    for (int i = 0; i < num_cmds; i++){
        if (strcmp(cmd, cmds[i].cmd) == 0){
            return cmds[i].fcn;
        }
    }
    return NULL;

}



    
