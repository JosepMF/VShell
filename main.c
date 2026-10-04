#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#include "define.h"
#include "exect.h"

int set_builtins(char* command_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE]);

int get_user_input(char buffer[MAX_BUFFER_SIZE]);

int process_input_buffer(char buffer[MAX_BUFFER_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE]);

int process_token_buffer(int n_args[MAX_COMMAND_SIZE], int* n_comand, char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE], int* has_infile, int* has_outfile, int* has_bgnd , int* has_append, int token_buffer_size, char** infile, char** outfile);

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

int get_user_input(char buffer[MAX_BUFFER_SIZE]) {
    int k = 0;

    printf("%s> ", getcwd(NULL, 0));

    int c;
    while(1) {
        c = getchar();

        // conition if the buffer reach the size limit
        if(k == MAX_BUFFER_SIZE - 1) {
            return ERROR;
        }

        // return if EOF
        if(c == EOF) return 2;

        // return OK if the comand has finished
        if(c == '\n') {
            buffer[k++] = '\0';
            return OK;
        }

        buffer[k++] = c;
    }
}

// split the buffer into strings
int process_input_buffer(char buffer[MAX_BUFFER_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE]) {
    char* cmd = strtok(buffer, " ");
    
    int size = 0;
    token_buffer[size++] = cmd; 

    while(cmd != NULL) {
        cmd = strtok(NULL, " ");
        token_buffer[size++] = cmd;
    }

    // size of elementos + final NULL 
    return size;
}

int process_token_buffer(int n_args[MAX_COMMAND_SIZE], int* n_comand, char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE], int* has_infile, int* has_outfile, int* has_bgnd , int* has_append, int token_buffer_size, char** infile, char** outfile) {
    // get the flags 
    for(int i = 0; i < token_buffer_size - 1; i++) {
        if(strcmp(token_buffer[i], "&") == 0) *(has_bgnd) = 1;
        if(strcmp(token_buffer[i], "<") == 0) *(has_infile) = 1;
        if(strcmp(token_buffer[i], ">") == 0 || strcmp(token_buffer[i], ">>") == 0) *(has_outfile) = 1;
        if(strcmp(token_buffer[i], ">>") == 0) *(has_append) = 1;
    }

    // get in-file & out-file
    for(int i = 0; i < token_buffer_size - 2; i++) {
        // in-file
        if(strcmp(token_buffer[i], "<") == 0) {
            if(i + 1 < token_buffer_size - 1) {
                *infile = token_buffer[i + 1];
            } else {
                return ERROR;
            }
        }

        // out-file
        if(strcmp(token_buffer[i], ">") == 0 || strcmp(token_buffer[i], ">>") == 0) {
            if(i + 1 < token_buffer_size - 1) {
                *outfile = token_buffer[i + 1];
            } else {
                return ERROR;
            }
        }
    }

    // separe comands
    int comand_index = 0;
    int args_index = 0;

    int k = 0;
    int is_prev_command = 0;
    while(1) {
        if(token_buffer[k] == NULL) {
            comand_matrix[comand_index][args_index] = NULL;
            n_args[comand_index] = args_index;
            if(is_prev_command == 1) comand_index++;
            break;
        }

        if(
            strcmp(token_buffer[k], ">") == 0 ||
            strcmp(token_buffer[k], ">>") == 0 ||
            strcmp(token_buffer[k], "<") == 0 ||
            strcmp(token_buffer[k], "|") == 0 ||
            strcmp(token_buffer[k], "&") == 0
        ) {
            
            n_args[comand_index] = args_index;

            comand_matrix[comand_index][args_index] = NULL;
            
            if(is_prev_command) {
                comand_index++;
                args_index = 0;
            }

            is_prev_command = 0;

            if(strcmp(token_buffer[k], "|") == 0 || strcmp(token_buffer[k], "&") == 0)
                k++;
            else
                k += 2;
            continue;
        }

        comand_matrix[comand_index][args_index] = token_buffer[k];

        args_index++;
        k++;
        is_prev_command = 1;
    }

    *n_comand = comand_index;

    return OK;
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