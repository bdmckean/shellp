#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellp.h"
const char * token_sep = " \r\n\\\t";


char ** parseline(char * line, int * num_args){
    char ** args = malloc(1024 * sizeof(char *));
    char * arg;
    int argnum = 0;

    for ( int i = 0; i < 1024; i ++){
        args[i] = (char *)malloc(1024);
        if (args[i] == NULL){
            printf("Malloc failure\n");
            exit(1);
        }
    }

    arg = strtok(line,token_sep);
    if(debug > 5) printf("pl-%s\n",arg);
    // Parse this string to pull out special shell characters
    //
    while ( arg != NULL){
        char this_str[1024];
        char new_str[1024];
        strcpy(this_str,arg);
        strcpy(new_str,"");
        int nindex = 0;
        int tindex = 0;
         if(debug > 5) printf("pl2-%s\n",arg);
        while(1){
            char c = this_str[tindex];
            if (debug > 10) {
                if ( debug > 6) printf("1char=%c, arg=%s, t=%s, n=%s, ti=%d, ni=%d\n",
                        c,arg,this_str, new_str, tindex, nindex);
            }
            if ( c == '|' || c == '<' || c == '>' || c == '(' || c == ')' ){
                if (nindex != 0){
                    new_str[nindex] = '\0';
                    if (debug > 5) printf("plx-%s\n",new_str);
                    strcpy(args[argnum],new_str); 
                    argnum++;
                    strcpy(new_str,"");
                    nindex = 0;
                }
                args[argnum][0] = c;
                args[argnum][1] = '\0';
                argnum++;
                tindex += 1;
                continue;
            } 
            new_str[nindex] = c;
            new_str[nindex+1] = '\0';
            if (debug > 10) {
                if (debug > 5) printf("2char=%c, arg=%s, t=%s, n=%s, ti=%d, ni=%d\n",
                        c,arg,this_str, new_str, tindex, nindex);
            }
            if (c == '\0') break;
            tindex++;
            nindex++;
            if (tindex > 1024) {
                printf("shellp error parsing line\n");
                exit(1);
            }
        }
        if ( debug > 6) printf("plx-%s argnum gs\n",new_str);
        if (strlen(new_str)){
            strcpy( args[argnum], new_str); 
            argnum++;
        }
        if ( debug > 5) printf("plx-%s argnum\n",new_str);
        arg = strtok(NULL,token_sep);
    }
    args[argnum] = NULL;
    return args;
} 

/*
    arg = strtok(line,token_sep);
    printf("pl-%s\n",arg);
    while ( arg != NULL){
        // Assemble < and > with filename
        if ( arg[0] == '<'){
            if (strlen(arg) == 1 ){ 
                char * tmp = strtok(NULL,token_sep);
                strcat(arg,tmp);
            } 
        }
        if ( arg[0] == '>'){
            if (strlen(arg) == 1 || (strlen(arg)==2 && arg[1] == '>')){ 
                char * tmp = strtok(NULL,token_sep);
                strcat(arg,tmp);
            }
        }
        // Put pipes on seperate line
        char * ret;
        if ((ret = strchr(arg,'|')) != NULL) {
            char * saveptr;
            if (debug > 11) printf("1=%s %d\n", arg, argnum);
            int add = 1;
            if (arg[0] == '|'){
                    args[argnum] = "|";
                    argnum++;
                    add = 0;
            } 
            part1 = strtok(arg,"|");
            if (debug > 11) printf("2=%s %d\n", arg, argnum);
            while (part1 != NULL ){
                args[argnum] = part1;
                argnum++;
                if (add){
                    args[argnum] = "|";
                    argnum++;
                    add = 0;
                }
                part1 = strtok(NULL,"|");
                if (strchr(arg,'|')) add = 1;
                if (debug > 11) printf("3=%s %d\n", arg, argnum);

           }
        }
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
    *num_args = argnum;
*/
