#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * token_sep = " \r\n\\\t";

#define argsize 128 

char ** parseline(char * line){

    char ** args = (char **) malloc(argsize*sizeof(line));
    char * arg;
    int argnum = 0;
   
    arg = strtok(line,token_sep);
    printf("pl-%s\n",arg);
    while ( arg != NULL){
        args[argnum] = arg;
        argnum++;
        if (argnum > argsize){
            fprintf(stderr,"too many arguments\n");
            exit (1);
        }
        arg = strtok(NULL,token_sep);
        printf("pl2-%s\n",arg);
    }
    args[argnum] = NULL;
    return args;

} 
