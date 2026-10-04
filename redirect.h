#ifndef REDIRECT_H
#define REDIRECT_H

#include "define.h"

typedef struct {
    int in;
    int out;
} ORDER_NET[MAX_COMMAND_SIZE];

void order_net_init();

int pipline(int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand);

int redirect_input(int i);

int redirect_output(int i);

int closeAll();

#endif