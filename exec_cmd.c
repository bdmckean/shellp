#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shellp.h"
#include "commands.h"

int (* fcn)( char ** args);

int exec_cmds(char ** cmd_list, int num_args, bool batch){

    int debug = 10;

    int background = 0; 
    if ( batch ) {
        // batch mode
        //
        if (debug > 9) printf("Batch Mode \n");
        printf("Batch Mode Not yet implemented\n");// fixme
        exit(0);    
    }

    char * cmd, **args = cmd_list;
    cmd = args[0];
    printf("%s:%d\n",cmd,num_args);
    if (cmd == NULL) return 0;
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
    // Interactive mode
    if (debug > 8) printf("cm:%s\n",cmd_list[0]);
    if (debug > 9) printf("Interactive Mode \n");
    
    int index = 0;
    bool more_cmds = true;
    if (debug > 8) printf("3.5:%s\n",cmd_list[0]);
  
    char infile[1024];
    char outfile[1024];
    char  * arglist[1024];
    for ( int i = 0; i < 1024; i ++){
        arglist[i] = malloc(sizeof(char)*1024);
    }

    while(more_cmds){
        char ** next_cmd = cmd_list + index;
        more_cmds = false;
        bool in = false, out = false, out_append = false;
        
        char * pipe_cmds[256];
        int num_pipe_cmds = 0;
        bool piping = false;
        int fdin = 0, fdout = 1; 
        int wr_flag = O_CREAT|O_WRONLY;
        int rd_flag = O_RDONLY;
        more_cmds = false;

        // Parse to the next ; or enda
        int arg = 1;
        int arg_index = 1;
        strcpy(arglist[0], next_cmd[0]);

        if (debug > 3) printf("3:%s mc=%d\n",arglist[0], (int)more_cmds);
        while(1){
            // We've reached the end
            if (next_cmd[arg] == NULL) break;
            // There is more
            if (debug > 3) printf("3:%s %d %s if=%s\n"
                    ,next_cmd[0], arg, next_cmd[arg], next_cmd[arg+1]);
            // Pull out pipes and redirects
            if (next_cmd[arg][0] == '<'){
                in = true;
                strcpy(infile,&(next_cmd[arg][1]));
                goto next_item;
            } else if (next_cmd[arg][0] == '>'){
                out = true;
                if ( next_cmd[arg][1] == '>'){
                    out_append = true;
                    wr_flag = O_CREAT|O_WRONLY|O_APPEND;
                    strcpy(outfile, &(next_cmd[arg][2]));
                } else {
                    wr_flag = O_CREAT|O_WRONLY;
                    strcpy(outfile, &(next_cmd[arg][1]));
                }
                goto next_item;
            } else if (next_cmd[arg][0] == '|'){
                piping = true;
                pipe_cmds[num_pipe_cmds] = next_cmd[arg];
                num_pipe_cmds += 1;
                goto next_item;
            }  
            if (next_cmd[arg][0] == ';'){
            // Or there is more Replace next ; with a NULL
                next_cmd[arg] = NULL;
                if (next_cmd[arg+1] != NULL){
                    more_cmds = true;
                    index = index + 1;
                }
                break;
            }
            printf("arglist[x]%s, next_cmd[y]%s\n",arglist[arg_index], next_cmd[arg]);
            strcpy(arglist[arg_index], next_cmd[arg]);
            arg_index = arg_index + 1;
next_item:
            arg = arg + 1;
            index = index + 1;
        }
        arglist[arg_index] = NULL;
        if (debug > 3) printf("5:%s mc=%d, in=%d, out=%d\n",
                    next_cmd[0], (int)more_cmds, (int)in, (int)out);
        if (debug > 8){
            char * tmp = arglist[0];
            int i = 0;
            while ( tmp != NULL){
                printf("%s:", tmp);
                i++;
                tmp = arglist[i];
                if ( tmp == NULL) printf("\n");
            }
        } 

        int num_args = arg;
        // Is this backgound?
        if ( next_cmd[num_args-1][0] == '&') {
            background = 1;
            next_cmd[num_args-1] = NULL;
        }
        else background = 0;

        bool passthru = false;
        if (debug > 3) printf("5.4:%s %s\n",arglist[0], arglist[1]);
        if ( (fcn = command_handler(arglist[0])) == NULL){
            // Shell does not execute this command
            passthru = true;
        } else {
            if (fcn(arglist) == 1) {
                // Shell does part of this command and passes the rest
                passthru = true; 
            }
        }
        if (!passthru) continue;
        
        if (debug > 3) printf("5.5:%s\n",arglist[0]);
        // Set up files and pipes
        if (in) fdin = open(infile,rd_flag, 0);
        if (fdin < 0 ) { 
            printf("fopen error in=%d, fn=%s\n", fdin, infile);
            return 1;
        }
        if (out) fdout = open("tmp",wr_flag,0644);
        if (fdout < 0) { 
            printf("fopen error out=%d, fn=%s\n", fdout, outfile);
            if (fdin > 0) close(fdin);
            return 1;
        }
        // Fixme 
        if (debug) printf("7 fdin%d fdout%d pipes%d \n",fdin, fdout, num_pipe_cmds);
        if (debug) printf("8 cmd:%s args%s bg:%d \n",arglist[0], arglist[1], background);

        // Execute commands in this line 
        int pid = fork();
        if ( pid < 0 ){
            printf("fork failed\n");
            exit(1);
        }
        if (pid  == 0) {
            // Child
            if (fdin != 0 ) { dup2(fdin,STDIN_FILENO);  close(fdin); }
            if (fdout != 1 ) {dup2(fdout,STDOUT_FILENO); close(fdout);}
            execvp(arglist[0], arglist); 
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
        if (fdin > 0 ) close(fdin);
        if (fdout > 1) close(fdout);
    }
    return 0;   
}
