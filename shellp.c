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

int (* fcn)( char ** args);

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
    char * cmd, **args;
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
        if (debug > 8){
            char * tmp = args[0];
            int i = 0;
            while ( tmp != NULL){
                printf("%s:", tmp);
                i++;
                tmp = args[i];
                if ( tmp == NULL) printf("\n");
            }
        } 
        bool passthru = false;
        if ( (fcn = command_handler(cmd)) == NULL){
                // Shell does not execute this command
                passthru = true;
        } else {
            if (fcn(args) == 1) {
                    // Shell does part of this command and passes the rest
                    passthru = true; 
            }
        }
        if (!passthru) continue;
        if ( args[num_args-1][0] == '&') {
            background = 1;
            args[num_args-1] = NULL;
        }
        else background = 0;

        int pid = fork();
        if ( pid < 0 ){
            printf("fork failed\n");
            exit(1);
        }
        if (pid  == 0) {
            // Child
            // execvp(fullpathname, args); } 
            execvp(cmd, args); 
        } else { 
            // Parent
            if (background){
                printf("Child process spawned pid=%d\n",pid); 
            } else {
                int status=0;
                wait(&status);
                printf("Child exited with status of %d/n", status); 
            } 
        }
    }
        
    exit (0);
}
