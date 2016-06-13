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

#define CMD_DONE 0
#define CMD_NOT_DONE 1



int init_dir_stack(void){
    char cwd[1024];
    char * cwdp = cwd;
    // Initialize directory stack
    if (getcwd(cwdp, sizeof(cwd)) != NULL){
        strcpy(dir_stack[0],cwdp);
        num_dirs = 1;
        next_dir = 1;
    } else {
        printf("call to getcwd returned NULL\n");
        exit(1);
    }
    return 0;
}
//
// Commands
//

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


int add_current_dir_to_stack(void){
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
    if ( debug > 2) printf("next_dir=%d,num_dir=%d\n",next_dir,num_dirs);
    if (next_dir == 0 ) this_dir = SIZE_DIR_STACK -1;
    else this_dir = next_dir - 1;
    for (int i = 0; i < num_dirs ; i++){
        if ( debug > 2 ) printf("dirstack[%d],%d,%s\n",this_dir,i,dir_stack[this_dir]);
        if (this_dir == 0) this_dir = SIZE_DIR_STACK -1;
        else this_dir--;
    }
    return 0;
}



int chg_dir(char ** args){
    if( debug > 21){
    printf("chd_dir ");
    fflush(stdout);
    if (debug) printf("cmd:%s , %s\n",args[0], args[1]);
    fflush(stdout);
    }
    if (args[1] == NULL || args[2] != NULL){
        fprintf(stderr, "shellp expected one argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0){
            perror("shellp");
        }
    }
    add_current_dir_to_stack();
   return CMD_DONE; 
}

int dh(char ** args)
{
    // Directory history
    //
    int n;
    char * tmp;

    if ( debug > 9) printf("Dir History \n");
    if ( num_dirs == 0){
        printf("No history\n");
        return 0;
    }

    tmp = args[1];
    if ( debug > 9) printf("History2 %s \n", tmp);
    if (tmp == NULL) {
        n = 20;
    } else if (validate_number(tmp) != 0){
       printf("shellp error - illegal fommand cound \n");
       return 1; 
    } else {
       n = atoi(args[1]);
    }
    int thisd = next_dir - 1; 
    if (thisd < 0 && num_dirs > 0) thisd = SIZE_DIR_STACK - 1;
    if (n > num_dirs) n = num_dirs; 
    if ( debug > 9) printf("History4 %d tc%d cc%d\n", n, thisd, this_dir); 
    for ( int i = 0; i < n; i ++ ){ 
        if ( debug > 9) printf("History5 %d tc%d cc%d\n", n, thisd, this_dir); 
        printf(" %d %s\n", i, dir_stack[thisd]); 
        thisd--; 
        if ( thisd < 0 ) thisd = SIZE_DIR_STACK - 1;  
    }



    return 0;

}

int dx (char ** args){
    char tmp[1024];
    
    if (num_dirs < 2) return 0;

    int thisd = next_dir - 1; 
    if (thisd < 0 && num_dirs > 0) thisd = SIZE_DIR_STACK - 1;
    int lastd = thisd - 1;
    if (lastd < 0 && num_dirs > 0) lastd = SIZE_DIR_STACK - 1;

    strcpy(tmp,dir_stack[lastd]);
    strcpy(dir_stack[lastd],dir_stack[thisd]);
    strcpy(dir_stack[thisd],tmp);

    if (chdir(tmp) != 0){
            perror("shellp");
    }
    
    return 0;
}

int dc (char ** args){
    char tmp[1024];
    char * tmp2;
    int n;

    tmp2 = args[1];
    if ( debug > 9) printf("DC %s \n", tmp2);
    if (tmp2 == NULL) {
        return 0;
    } else if (validate_number(tmp) != 0){
       printf("shellp error - illegal fommand cound \n");
       return 1; 
    } else {
       n = atoi(args[1]);
    }

    if (n > num_dirs) return 0;

    int thisd = next_dir - n - 1; 
    if (thisd < 0 && num_dirs > 0) thisd = SIZE_DIR_STACK - 1;


    if (chdir(dir_stack[thisd]) != 0){
            perror("shellp");
    }
    add_current_dir_to_stack();
    
    return 0;
}
struct cmd_fcn cmds[] =  
    {
        {"cd", chg_dir},
        {"quit", quit},
        {"exit", quit},
        {"dx", dx},
        {"dh",dh},
        {"dc",dc},
        {"fc", fc},
        {"history", history}
    };



int (* (command_handler(char * cmd)))(char ** args){
    int num_cmds = sizeof(cmds)/sizeof(struct cmd_fcn);

    if (debug > 20 ) {
        if ( debug > 5) printf("cmd_h %s", cmd);
        fflush(stdout);
    }   
    for (int i = 0; i < num_cmds; i++){
        if (strcmp(cmd, cmds[i].cmd) == 0){
            return cmds[i].fcn;
        }
    }
    return NULL;
} 

    
