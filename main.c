#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#include "define.h"
#include "exect.h"
#include "inptprcs.h"


int set_builtins(char* command_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE]);

int main(void) {

    int running = 1;
    while(running) {
        // sesion variables
        
        // buffer de comando
        char buffer[MAX_BUFFER_SIZE];
    
        // tokenized buffer
        char* token_buffer[MAX_TOKEN_BUFFER_SIZE];
        // commands matrix
        char* command[MAX_COMMAND_SIZE];
        char* command_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE];

        int n_command = 0;
        int n_args[MAX_COMMAND_SIZE];

        // in-file, out-file
        char* infile = NULL;
        char* outfile = NULL;

        // in-out files or bgnd mode
        int has_infile = 0, has_outfile = 0, has_bgnd = 0, has_append = 0; 
        
        // functionality

        // show prompt and get the user input
        int user_input_status = get_user_input(buffer);
        if(user_input_status == -1) {
            printf("Max command size reached\n");
            continue;
        } else if(user_input_status == 2) {
            running = 0;
            break;
        }

        // proccess de input buffer
        int token_buffer_size = process_input_buffer(buffer, token_buffer);

        // process tokens
        if(process_token_buffer(n_args, &n_command, command_matrix, token_buffer, &has_infile, &has_outfile, &has_bgnd, &has_append, token_buffer_size, &infile, &outfile) == ERROR) {
            printf("Instruction wrong written\n");
            continue;
        }

        // builtins
        int builtin_ret = set_builtins(command_matrix);
        if(builtin_ret == CONT || builtin_ret == ERROR) continue;
 
        // execute
        execute(n_args, command_matrix, has_bgnd, has_infile, has_outfile, has_append, infile, outfile, n_command);
    }

    return 0;
}

// built ins set
int set_builtins(char* command_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE]) {
    if(strcmp(command_matrix[0][0], "cd") == 0 && command_matrix[0][1] != NULL) {
        if(chdir(command_matrix[0][1]) == ERROR) {
            perror("cd");
            return ERROR;
        }
        return CONT;
    }

    return OK;
}