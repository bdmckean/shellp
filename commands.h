#ifndef COMMANDS_H
#define COMMANDS_H


extern int (*(command_handler(char * cmd)))(char ** args);
extern int init_dir_stack(void);


#endif
