# Personal Shell Project

## Description

This is a personal shell project aimed at researching and understanding POSIX processes and system functionality by building a shell in C.

The main goal is to build a shell inspired by Bash, where users can execute commands interactively from a prompt and eventually run shell scripts.

I also want to use this project to gain more experience with operating systems, learn how to organize and maintain a large codebase, and improve my software development skills and portfolio.

If I find that the project can grow beyond its initial scope, I would like to turn this repository into a more serious open-source shell project and encourage contributions from other developers.

## Functionality

The shell receives input from the user through an interactive prompt. This input is then parsed into commands, input files, output files, and other flags.

Commands are organized into a 2D matrix, while features such as pipes and I/O redirections are managed through a data structure that determines how the standard input and output of each command should be connected.

Once the pipeline and redirection configuration has been established, the shell creates child processes and executes the corresponding programs.

If the background flag is not enabled, the parent process waits for all child processes to finish before displaying the prompt again. If the background flag is enabled, the shell does not wait for the child processes and immediately returns control to the user.

This is only a very superficial overview of how the shell works. I will update this README with a more detailed description as the project develops.

## Help and Contributions

I appreciate any help, feedback, or comments that can help me improve this project and learn more about how operating systems and Unix processes work.

If you are interested in contributing, I am open to suggestions, ideas, and improvements.
