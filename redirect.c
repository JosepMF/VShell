#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>

#include "redirect.h"
#include "define.h"

ORDER_NET order_net;

void order_net_init() {
    for(int i = 0; i < MAX_COMMAND_SIZE; i++) {
        order_net[i].in = STDIN_FILENO;
        order_net[i].out = STDOUT_FILENO;
    }
}

int pipline(int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand) {
    order_net_init();

    int in = STDIN_FILENO, out = STDOUT_FILENO;

    // openning /dev/null in bgnd mode
    if(has_bgnd) {
        in = open("/dev/null", O_RDONLY);
        if(in == ERROR) {
            printf("Error opnenning /dev/null\n");
            return ERROR;
        }
    }

    // openning infile in input file mode
    if(has_infile) {
        in = open(infile, O_RDONLY);
        if(in == ERROR) {
            printf("Error openning %s\n", infile);
            return ERROR;
        }
    }

    // openning outfile in output file mode
    if(has_outfile) {
        int mode = O_TRUNC;

        if(has_append) mode = O_APPEND;

        out = open(outfile, O_CREAT | O_WRONLY | mode, 0644);

        if(out == ERROR) {
            printf("Error openning %s\n", infile);
            return ERROR;
        }
    }

    order_net[0].in = in;
    order_net[n_comand - 1].out = out;

    // connecting pipes
    for(int i = 0; i < n_comand - 1; i++) {
        int fd[2];
        if(pipe(fd) == ERROR) {
            printf("Error creating piplining");
            return ERROR;
        }

        order_net[i + 1].in = fd[0];
        order_net[i].out = fd[1];
    }

    return OK;
}

int redirect_input(int i) {
    if(order_net[i].in == STDIN_FILENO) return OK;

    if(dup2(order_net[i].in, STDIN_FILENO) == ERROR) {
        printf("Error editing file descriptor (input) in process %d with pid %ld\n", i, (long) getpid());
        return ERROR;
    }

    return OK;
}

int redirect_output(int i) {
    if(order_net[i].out == STDOUT_FILENO) return OK;

    if(dup2(order_net[i].out, STDOUT_FILENO) == ERROR) {
        printf("Error editing file descriptor (output) in process %d  with pid %ld\n", i, (long) getpid());
        return ERROR;
    }
    
    return OK;
}

int closeAll() {
    for(int i = 0; i < MAX_COMMAND_SIZE; i++) {
        if(order_net[i].in == STDIN_FILENO) continue;

        if(close(order_net[i].in) == ERROR) {
            printf("Error closing the input file at process %ld\n", (long) getpid());
            return ERROR;
        }
    }

    for(int i = 0; i < MAX_COMMAND_SIZE; i++) {
        if(order_net[i].out == STDOUT_FILENO) continue;

        if(close(order_net[i].out) == ERROR) {
            printf("Error closing the output file at process %ld\n", (long) getpid());
            return ERROR;
        }
    }
    
    return OK;
}
