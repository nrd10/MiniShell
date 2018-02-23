Minitiarized Version of a Command Shell
=======================================

The source code can be compiled to build a program that works as
a minitarized version of a command shell. The Makefile provided
can be used to compile an executable. The program requires installation
of a GNU c++ compiler.

This program can run the majority of commands that you would type into the command
line. It can run any executable file specified and handles
command argumentsand paths that are both relative and absolute. 
If a path does not containa / it will search only in the specified 
directory.

The shell is able to handle the "cd" command allowing you to switch
back and forth from directories. Additionally it handles two commands
that can switch variables, set var value, and export var. set var value
will add a variable and allow the shell to remember it. Export var 
will put the current value of var into th environment for other programs.
Missing functionalities include opening text editors like emacs or vim,
and using pipes. Additionally, ater a specified program exits the shell prints:

Program exited with status 0
[replace 0 with the actual exit status]
OR
Program was killed by signal 11
[replace 11 with the actual signal]

If a command is not found a message will print:

Command commandName not found
[replace commandName with the actual command name]
