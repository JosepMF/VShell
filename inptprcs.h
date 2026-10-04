#ifndef INPTPRCS_H
#define INPTPRCS_H

#include "define.h"

int get_user_input(char buffer[MAX_BUFFER_SIZE]);

int process_input_buffer(char buffer[MAX_BUFFER_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE]);

int process_token_buffer(int n_args[MAX_COMMAND_SIZE], int* n_comand, char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE], int* has_infile, int* has_outfile, int* has_bgnd , int* has_append, int token_buffer_size, char** infile, char** outfile);

#endif