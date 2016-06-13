#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "commands.h"
#include "shellp.h"

#define SIZE_CMD_STACK 100
char * cmd_stack[SIZE_CMD_STACK];
#define MAX_CMD_SIZE 1024

int cur_cmd;
int next_cmd;
int num_cmds;

int init_cmd_stack(void){
    for (int i = 0; i < SIZE_CMD_STACK; i++){
        cmd_stack[i] = malloc(sizeof(char)*1024);
        if (cmd_stack[i] == NULL){
            printf("Malloc failed\n");
            exit(1);
        }
    }
    cur_cmd = 0;
    next_cmd = 0;
    num_cmds = 0;
    return 0;
}

int add_cmd(char * cmd){
    if ( debug > 9) printf("add cmd %s\n", cmd);
    cur_cmd = next_cmd;
    strcpy(cmd_stack[cur_cmd],cmd);
    cmd_stack[cur_cmd][strcspn(cmd_stack[cur_cmd],"\n")] = '\0';
    next_cmd++;
    if ( next_cmd == SIZE_CMD_STACK ) next_cmd = 0;
    if ( num_cmds < SIZE_CMD_STACK ) num_cmds++;
    if ( debug > 9) printf("cur=%d, nxt=%d, cmd=%s\n",
          cur_cmd, next_cmd, cmd);
    return 0;  
}

int validate_number(char * n){
    if ( n == NULL) return 1;
    if ( debug > 10) printf("validat2 Num=%s\n", n);
    fflush(stdout);
    for (int i = 0; i < strlen(n); i++){
        if ( !isdigit(n[i])) {
            if ( debug > 10) printf("validat2 Num=%s, digit%c\n", n, n[i]);

                return 1;
         }
    }
    return 0;
}

int history(char ** args){
    // show n commands
    int n;
    char * tmp;
    
    if ( debug > 9) printf("History \n");
    if ( num_cmds == 0){
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
    if ( debug > 9) printf("History3 %d \n", n);
    int thisc = cur_cmd;
    if (n > num_cmds) n = num_cmds;
    if ( debug > 9) printf("History4 %d tc%d cc%d\n", n, thisc, cur_cmd);
    for ( int i = 0; i < n; i ++ ){
        if ( debug > 9) printf("History5 %d tc%d cc%d\n", n, thisc, cur_cmd);
        printf(" %d %s\n", i, cmd_stack[thisc]);
        thisc--;
        if ( thisc < 0 ) thisc = SIZE_CMD_STACK - 1; 
    }
    return 0;
}

int fc(char ** args){
    char * tmp1, * tmp2;
    char ** cmd_args;
    tmp1 = args[1];
    tmp2 = args[2]; 
    int start, end;

    if ( debug > 10) printf("fc %s, %s\n", tmp1, tmp2);
    if (tmp1 == NULL && tmp2 == NULL){
        start = cur_cmd;
        end = cur_cmd;
    } else if ( tmp2 == NULL ) {
        if (validate_number(tmp1) != 0){
            printf("shellp error - illegal fommand found \n");
            return 1; 
        } else {
            start = atoi(tmp1);
            end = start;
        }
    } else if (validate_number(tmp1) == 0 && validate_number(tmp2) == 0){
            start = atoi(tmp1);
            end = atoi(tmp2);
    } else {
        printf("shellp error - xillegal fommand found \n");
        return 1;
    }

    if ( debug > 10) printf("fc2 %d, %d\n", start, end);
    if (start < 0 || start > 1023 || end < 0 || end > 1023 || start > end ){
            printf("shellp error - illegal fommand found \n");
            return 1; 
    }
    int length = end - start + 1;
    if (length > num_cmds) {
            printf("shellp error - too many commands requested \n");
            return 1; 
    }
    start = cur_cmd - 1 - start;
    if ( start < 0 ) start = SIZE_CMD_STACK+start;
    end = cur_cmd -1  - end;
    if ( end < 0 ) end = SIZE_CMD_STACK+start;

    int thisc = start;
    for (int i = 0; i < length; i++){
        char next_cmdp[1024];
        strcpy(next_cmdp,cmd_stack[thisc]);
        char * cmd;
        int num_args;

        if (debug > 10){
            int len = num_cmds;
            for ( int k = 0; k < len; k++ ){
                printf("i=%d cmd=%s\n", k, cmd_stack[k]);
            }
        }


        if ( debug > 10) printf("fc3 %s, %d %d\n", next_cmdp,  i, length);
        cmd_args = parseline(next_cmdp, &num_args);
        cmd = cmd_args[0];
        if ( debug > 10) printf("fc4 %s %d %d %d\n", cmd, start, end, thisc);
        if ( debug > 10) printf("fc4.1 %d %d %d\n", cur_cmd, next_cmd, num_cmds);
        if (debug > 11) printf("fc5%s:%d\n",cmd,num_args);
        if (cmd != NULL) {
            if ( exec_cmds(cmd_args, num_args, false) != 0 ){
                exit(1);
            }
        }
        for ( int j = 0; j < 1024; j++ ){
            free(cmd_args[j]);
        }
        free(cmd_args);
        thisc--;
        if ( thisc  < 0   ) thisc = SIZE_CMD_STACK - 10;
    } 
    return 0; 

}

