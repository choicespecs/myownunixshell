Project 4: yosh

Created by:
Christopher Lee

1. Introduction
-------------------
This is a shell that is created using readline GNU library as well as utilizing functions created within parse.c. This shell has a few built in commands such as jobs, history, kill, cd, exit.
and help. This is a very extensive project which includes several different functions within it. I will do my best to highlight the most important functions that allow the shell to execute 
and run. 

A few notes however
1) This shell's job function does not fully mimic a typically jobs unix process. If a process is killed or finished. It will simply be removed from the process list without being notified.
2) I've tested for many different cases, but there could possibly be a few circumstances where the shell may break. If this is the case then I would certainly like to try and improve it for the future.

2. Installation.
--------------------
This file will require the following files for installation.

    a) parse.c
    b) parse.h
    c) yosh.c
    d) yosh_include.h
    e) yosh_help.txt (this file has all the help menu can be replaced and modified this way).

A makefile has been provided to assist with the installation of the sfind file (as well as
included the functionality to remove all relevant files). The yosh.c file contains many of the
functions and code for the shell. I wasn't able to quite split the functions for this project, but
i'd definitely am interested in working on this and continuing to process and improve on this shell.

If any problems occur with the makefile compilation, you should be able to create an executable
using the following command with gcc:
    "gcc parse.c yosh.c -o yosh"


3. Files / Methods
--------------------
The chunk file is separated only into two main files.

    a) parse.c
    b) yosh.c

a) parse.c
    this file contains a bulk of the skeleton code which allows the shell to process information from the cmdline such as redirection, background, input, output, etc. through the use of 
    an additional struct called parseInfo.

b) yosh.c
    this file contains a majority of the code that has much of the implementation of the shell. Yosh will have a few main functions.

    a) execute_built_in_commands() & is_builtin_commands()
    both of these functions will utilize many functions within the shell to work with functions such as jobs, hisotry, kill, cd, and exit.

    b) pipeline()
    function which allows for recursive piping within the shell.

    c) tilde_expansion_remix & create_wildcard_remix()
    both functions will allow tilde expansion and wildcard within this shell. I manually replace the ~ to the home directory and I also use glob() for the wildcard.

    d) setup_info() & setup_info_with_pipes()
    both functions will setup input, output redirection possibly within the shell, also will prepare functions for signals etc.

4. References
----------------
Recursive piping:

https://stackoverflow.com/questions/21307013/recursive-piping-in-unix-again
https://stackoverflow.com/questions/35143797/multiple-pipe-recursive-handing-in-c
https://www.unix.com/programming/192651-c-unix-pipes-fork-recursion.html
https://stackoverflow.com/questions/34461927/recursive-function-running-multiple-pipes

tilde expansion and wildcard
https://stackoverflow.com/questions/32276909/why-is-a-tilde-in-a-path-not-expanded-in-a-shell-script
https://stackoverflow.com/questions/3963716/how-to-manually-expand-a-special-variable-ex-tilde-in-bash

glob()
https://man7.org/linux/man-pages/man3/globfree.3.html
    
