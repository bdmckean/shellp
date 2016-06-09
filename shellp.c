#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellp.h"
#include "commands.h"

const char * default_prompt = "shellp>";
char prompt[1024];
int debug = 10;
int background;
int num_args;



int main(int argc, char ** argv){
    background = 0; 
    if ( argc > 1 ) {
        // batch mode
        //
        if (debug > 9) printf("Batch Mode \n");
        printf("Batch Mode Not yet implemented\n");// fixme
        exit(0);    
    }

    // Interactive mode
    if (debug > 9) printf("Interactive Mode \n");

    init_dir_stack();
    strcpy(prompt,default_prompt);
    char hostname[1024];
    hostname[1023] = '\0';
    if (gethostname(hostname, 1023) == 0){
        strcpy(prompt,hostname);
        strcat(prompt,">");
    }
    char next_cmd[1024];
    char * next_cmdp = next_cmd;
    char *cmd, **args;
    while(1){
        // Top of Interactive Shell 
        printf("%s",prompt);
        fgets(next_cmdp, sizeof(next_cmd), stdin);
        //printf("%s",next_cmd);
        if (strlen(next_cmdp) == 0) continue;
        
        args = parseline(next_cmdp, &num_args);
        cmd = args[0];
        printf("%s:%d\n",cmd,num_args);
        if (cmd == NULL) continue;
       
        if ( exec_cmds(args, num_args, false) != 0 ){
            exit(1);
        }

    }
        
    exit (0);
}
