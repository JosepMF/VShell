#ifndef EXECT_H
#define EXECT_H

#include "define.h"

void execute(int n_args[MAX_COMMAND_SIZE], char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand);

#endif