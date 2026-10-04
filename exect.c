#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "exect.h"
#include "redirect.h"

void execute(int n_args[MAX_COMMAND_SIZE], char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand) {
    // load pipes and redirects in the file descriptor 
    if(pipline(has_bgnd, has_infile, has_outfile, has_append, infile, outfile, n_comand) == ERROR) {
        printf("Error in pipline\n");
        return;
    }
    
    // set the processes
    for(int i = 0; i < n_comand; i++) {
        pid_t ret = fork();
        switch(ret) {
            case ERROR:
            printf("Fatal error creating the process\n");
            break;
            case 0:
            redirect_input(i);
            redirect_output(i);
            closeAll();
            if(execvp(comand_matrix[i][0], comand_matrix[i]) == ERROR) {
                printf("An error has ocurred in the execution of file %s\n", comand_matrix[0][0]);
                exit(0);
            }
            break;
            default:
            break;
        }
    }

    closeAll();
    
    if(!has_bgnd) {
        // wating all childs
        for(int i = 0; i < n_comand; i++) {
            if(wait(NULL) == ERROR) {
                printf("An error was ocurred waiting a child process\n");
            }
        }
    }
}