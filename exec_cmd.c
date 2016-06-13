#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "shellp.h"
#include "commands.h"



int (* fcn)( char ** args);
int start_cmd(int fdin, int fdout, char ** arglist, int background, int close_this);

int exec_cmds(char ** cmd_list, int num_args, bool batch){
    char * cmd;
    char ** args;
    int background = 0;


    args = cmd_list;
    cmd = args[0];
    if ( debug > 2) printf("sc1 %s:%d\n",cmd,num_args);
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
    if (debug > 8) printf("cm:%s\n",cmd_list[0]);
    
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
        
        int pipe_cmds[256];
        int pipe_cmds2[256];
        int num_pipe_cmds = 0;
        int num_pipe_cmds2 = 0;
        int piping = 0;
        int piping2 = 0;
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
            // Have we reached the end
            if (next_cmd[arg] == NULL) break;
            // There is more
            if (debug > 3) printf("3.2:%s %d %s if=%s\n"
                    ,next_cmd[0], arg, next_cmd[arg], next_cmd[arg+1]);
            // Pull out pipes and redirects
            if (next_cmd[arg][0] == '<'){
                if (next_cmd[arg+1][0] != '(') {
                    in = true;
                    strcpy(infile,next_cmd[arg+1]);
                    // skip the next item
                    arg = arg + 1;
                    goto next_item;
                } else { // found "<("
                    // Process subsition using named pipes
                    if (debug > 3) printf("3.7:%s:%s:%s\n",
                            next_cmd[arg],next_cmd[arg+1], next_cmd[arg+2]);
                    piping2 = 2;
                    pipe_cmds2[num_pipe_cmds2] = arg_index+1;
                    num_pipe_cmds2 += 1;
                    arglist[arg_index] = NULL;
                    arg_index = arg_index + 1;
                    arg = arg + 2;
                } 
            } else if (next_cmd[arg][0] == '>'){
                out = true;
                if ( next_cmd[arg+1][0] == '>'){
                    out_append = true;
                    wr_flag = O_CREAT|O_WRONLY|O_APPEND;
                    strcpy(outfile, next_cmd[arg+2]);
                    arg = arg + 2;
                } else { 
                    wr_flag = O_CREAT|O_WRONLY;
                    strcpy(outfile, next_cmd[arg+1]);
                    arg = arg + 1;
                }                 
                if ( debug > 3) printf("argl[x]%s, next_c[y]%s arg%d, arg_index %d of=%s\n"
                    ,arglist[arg_index], next_cmd[arg], arg, arg_index, outfile);
                goto next_item;
            } else if (next_cmd[arg][0] == '|'){
                piping = 1;
                pipe_cmds[num_pipe_cmds] = arg_index+1;
                num_pipe_cmds += 1;
                next_cmd[arg] = NULL;
                arglist[arg_index] = NULL;
                arg_index = arg_index + 1;
                goto next_item;
            } else if (next_cmd[arg][0] == ';'){
            // Or there is more Replace next ; with a NULL
                next_cmd[arg] = NULL;
                if (next_cmd[arg+1] != NULL){
                    more_cmds = true;
                    index = index + 1;
                }
                break;
            } else if ( next_cmd[arg][0] == ')'){
                if (debug > 3) printf("3.8:%s:%s:%s\n",
                        next_cmd[arg],next_cmd[arg+1], next_cmd[arg+2]);
                // end of a process subsitition command
                next_cmd[arg] = NULL;
                arglist[arg_index] = NULL;
                arg_index = arg_index + 1;
                goto next_item;
            }

            strcpy(arglist[arg_index], next_cmd[arg]);
            if ( debug > 3) printf("arglist[x]%s, next_cmd[y]%s arg%d, arg_index %d\n"
                    ,arglist[arg_index], next_cmd[arg], arg, arg_index);
            fflush(stdout);
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

        int num_args = arg_index;
        while ( arglist[num_args-1] == NULL) {num_args--;}
        // Is this backgound?
        if ( arglist[num_args-1][0] == '&') {
            background = 1;
            arglist[num_args-1] = NULL;
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
            if (piping) { printf("Error -- trying to pipe with shell commands \n");}
        }
        if (!passthru) continue;
        
        if (debug > 3) printf("5.5:%s\n",arglist[0]);
        // Set up files and pipes
        if (in) fdin = open(infile,rd_flag, 0);
        if (fdin < 0 ) { 
            printf("fopen error in=%d, fn=%s\n", fdin, infile);
            return 1;
        }
        if (out) fdout = open(outfile,wr_flag,0644);
        if (fdout < 0) { 
            printf("fopen error out=%d, fn=%s\n", fdout, outfile);
            if (fdin > 0) close(fdin);
            return 1;
        }
        // Fixme 
        if (debug) printf("7 fdin%d fdout%d pipes%d \n",fdin, fdout, num_pipe_cmds);

        // Execute commands in this line
        if (!piping && !piping2) {
            start_cmd(fdin,fdout,arglist,background, 0);
        } else if (piping ){
                // Normal pipes
                int in = fdin;
                int fd[2];
                // Create a pipe between each command
                char ** thislist = arglist;
                if (debug) printf("9 cmd:%s args%s bg:%d p?=%d np=%d\n",
                    thislist[0], thislist[1], background, (int)piping, num_pipe_cmds);
                for (int i = 0; i < num_pipe_cmds ; i++){
                    pipe(fd); 
                    start_cmd(in,fd[1],thislist,true, fd[0]);
                    close(fd[1]);
                    in = fd[0]; 
                    thislist = &(arglist[pipe_cmds[i]]);
                }
                start_cmd(in,fdout,thislist,background,0);
                piping = 0;
        } else {
            // Piping2  Named pipes
            char ** thislist = arglist;
            if (debug) printf("9.1 cmd:%s args%s bg:%d p?=%d np=%d\n",
                    thislist[0], thislist[1], background, (int)piping2, num_pipe_cmds2);
            char * tmparglist[10];
            for (int k = 0; k < num_pipe_cmds2+1; k++){
                tmparglist[k] = malloc(sizeof(char)*1024);
            }
            strcpy(tmparglist[0],thislist[0]);   
            printf("Here1\n");   
            // Make named pipes
            // Attach named pipe(s) to first command
            char pname[num_pipe_cmds2][256];
            int fnpr[num_pipe_cmds2];
            int fnpw[num_pipe_cmds2];
            printf("Here2\n");  
            for (int i = 0; i < num_pipe_cmds2 ; i++){
                strcpy(pname[i],"/tmp/npipe___");
                pname[i][11]=0x30+i;
                printf("Here3\n");
                int res;   
                if ( (res = mkfifo( pname[i], 0600 )) != 0) {
                    if ( errno == EEXIST ) {
                        if (debug > 2) printf("FIFO EXISTS\n");
                    } else{
                        printf("FIFO failed\n");
                        exit(1);
                    }
                }
                printf("Here4\n");  
                fnpr[i] = open(pname[i], O_RDWR);
                fnpw[i] = open(pname[i], O_WRONLY);
                strcpy(tmparglist[i+1],pname[i]);
                if (debug> 7) printf("9.9 fnpr=%d, fnpw=%d\n",
                        fnpr[i], fnpw[i]);
            }
            tmparglist[num_pipe_cmds2+1] = NULL;
            if (debug> 7) printf("9.4 cmd:%s %s %s\n",
               tmparglist[0], tmparglist[1], tmparglist[2]);

            start_cmd(in,out,tmparglist,true,0);
            // Attach other commands to write end of pipe(s)
            for (int i = 0; i < num_pipe_cmds2 ; i++){
                thislist = &(arglist[pipe_cmds2[i]]);
                if (debug) printf("9.2 cmd:%s args%s bg:%d p?=%d np=%d\n",
                    thislist[0], thislist[1], background, (int)piping2, num_pipe_cmds2);
                start_cmd(in,fnpw[i],thislist,background,0);
            }
            for (int k = 0; k < num_pipe_cmds2+1; k++){
                free(tmparglist[k]);
                if ( k < num_pipe_cmds) {
                    close(fnpr[k]);
                    close(fnpw[k]);
                    unlink(pname[k]);
                }   
            }

        }
        

        if (fdin > 0 ) close(fdin);
        if (fdout > 1) close(fdout);
        fflush(stdout);
    }

    for ( int i = 0; i < 1024; i ++){
        free(arglist[i]);
    }
    return 0;   
}


int start_cmd(int fdin, int fdout, char ** arglist, int background, int close_this){
    if (debug > 2) printf("Startcmd cmd:%s args%s bg:%d fdi=%d fdo=%d c=%d\n"
            ,arglist[0], arglist[1], background, fdin, fdout, close_this);
    int pid = fork();
    
    if ( pid < 0 ){
        printf("fork failed\n");
        exit(1);
    } else
    if (pid  == 0) {
        // Child
        if (debug > 10) printf("Startcmd (ch) cmd:%s args%s bg:%d fdi=%d fdo=%d c=%d\n"
            ,arglist[0], arglist[1], background, fdin, fdout, close_this);
        if (fdin != 0 ) { dup2(fdin,STDIN_FILENO);  close(fdin); }
        if (fdout != 1 ) { dup2(fdout,STDOUT_FILENO); close(fdout);}
        if ( close_this != 0 ) close(close_this);
        return execvp(arglist[0], arglist); 
    } else { 
        // Parent
        if (debug > 10) printf("Startcmd (p) cmd:%s args%s bg:%d fdi=%d fdo=%d c=%d\n"
            ,arglist[0], arglist[1], background, fdin, fdout, close_this);
        if (background){
            if (debug > 1) printf("Child process spawned pid=%d\n",pid); 
        } else {
            int status=0;
            wait(&status);
            if (debug > 1) printf("Child exited with status of %d/n", status); 
        } 
        if (debug > 10) printf("Startcmd (pe) cmd:%s args%s bg:%d fdi=%d fdo=%d c=%d\n"
            ,arglist[0], arglist[1], background, fdin, fdout, close_this);
        return (pid);
    }
    return 0;
}
