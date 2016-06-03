#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include "shellp.h"

int debug = 10;

int main(int argc, char ** argv){
    
    if ( argc > 1 ) {
        // batch mode
        //
        if (debug > 9) printf("Batch Mode \n");
        exit(0);    
    }

    // Interactive mode
    if (debug > 9) printf("Interactive Mode \n");

    char  prompt[sizeof(default_prompt)];
    strcpy(prompt,default_prompt);

    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    printf("Hostname: %s\n", hostname);

    printf("%s",prompt);    
    exit (0);
}
