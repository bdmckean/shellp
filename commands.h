#ifndef COMMANDS_H
#define COMMANDS_H


extern int (*(command_handler(char * cmd)))(char ** args);
extern int init_dir_stack(void);


extern int history(char ** args);
extern int fc(char ** args);
extern int init_cmd_stack(void);
extern int add_cmd(char * cmd);

#endif
