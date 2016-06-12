#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "shellp.h"
#include "commands.h"

const char * default_prompt = "shellp>";
char prompt[1024];
int debug = 0;
int background;
int num_args;



int main(int argc, char ** argv){
    background = 0;

    if ( argc > 1 ) {
        // batch mode
        //
        if (debug > 9) printf("Batch Mode %s %s\n", argv[0], argv[1] );
        // Open file, read commands, execute each one
        char * this_line = (char *)malloc(1024 * sizeof(char));
        FILE * fp;
        size_t size;
        if (debug > 9) printf("2Batch Mode \n");
        if ( (fp = fopen(argv[1],"r")) == NULL ){
            printf("Error opening file %s\n",argv[1]);
            exit(1);
        }
        fflush(stdout);
        while ( getline(&this_line, &size, fp) > 0 ){
            char ** args;
            int num_args;
            // remove CR
            if (debug > 9) printf("3Batch Mode \n");
            char * t = strtok(this_line,"\n");
            if (debug > 8) {
                printf(":line-%s\n",this_line);
                printf(":t-%s\n",t);
            }
            // echo line
            printf("%s\n",this_line);
            args = parseline(t,&num_args);

            if ( exec_cmds(args, num_args, true) != 0 ){
                exit(1);
            }
            for ( int i = 0; i < 1024; i++ ){
                free(args[i]);
            }
            free(args);
        }
        if (debug > 9) printf("4Batch Mode \n");
        exit(0);    
    }

    // Interactive mode
    if (debug > 9) printf("Interactive Mode \n");

    init_dir_stack();
    init_cmd_stack();
    strcpy(prompt,default_prompt);
    char hostname[1024] = "shellp-";
    hostname[1023] = '\0';
    if (gethostname(hostname, 1023) == 0){
        strcat(prompt,hostname);
        strcat(prompt,">");
    }
    char next_cmd[1024];
    char * next_cmdp = next_cmd;
    char *cmd, **args;
    while(1){
        // Top of Interactive Shell 
        printf("%s",prompt);
        if (fgets(next_cmdp, sizeof(next_cmd), stdin) == NULL){
            // EOF, ctrdD
            exit(0);
        }
        add_cmd(next_cmdp); 
        if (strlen(next_cmdp) == 0) continue;
        args = parseline(next_cmdp, &num_args);
        cmd = args[0];
        if (debug > 15) printf("%s:%d\n",cmd,num_args);
        if (cmd == NULL) continue;
       
        if ( exec_cmds(args, num_args, false) != 0 ){
            exit(1);
        }
        for ( int i = 0; i < 1024; i++ ){
            free(args[i]);
        }
        free(args);
    }
        
    exit (0);
}
