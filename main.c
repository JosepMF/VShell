#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_TOKEN_BUFFER_SIZE 100
#define MAX_COMMAND_SIZE 10
#define MAX_COMMAND_ARGS_SIZE 10

#define OK 1
#define CONT 4
#define ERROR -1

typedef struct {
    int in;
    int out;
} ORDER_NET[MAX_COMMAND_SIZE];
ORDER_NET order_net;

int set_builtins(char* command_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE]);

int get_user_input(char buffer[MAX_BUFFER_SIZE]);

int process_input_buffer(char buffer[MAX_BUFFER_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE]);

int process_token_buffer(int n_args[MAX_COMMAND_SIZE], int* n_comand, char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], char* token_buffer[MAX_TOKEN_BUFFER_SIZE], int* has_infile, int* has_outfile, int* has_bgnd , int* has_append, int token_buffer_size, char** infile, char** outfile);

int pipline(int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand);

void order_net_init();

void execute(int n_args[MAX_COMMAND_SIZE], char* comand_matrix[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS_SIZE], int has_bgnd, int has_infile, int has_outfile, int has_append, char* infile, char* outfile, int n_comand);

int redirect_input(int i);

int redirect_output(int i);

int closeAll();

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