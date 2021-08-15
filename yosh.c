
/*-----------------------------------------------------------------------------
FILE: yosh.c

NAME: Christopher Lee cl24327 Project 4 shell 

DESCRIPTION: YOSH - Shell
-------------------------------------------------------------------------------*/
#include "yosh_include.h"
#include "parse.h"   // local file include declarations for parse-related structs
#include <glob.h>

// GLOBAL VARIABLES //
int * background_process;   
char ** background_process_arguments;
int * process_number;
int process_size = 10;
int proceses = 0;
int std_out_fd;
int std_in_fd;
int process_number_id = 1;
enum BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT, JOBS, CD, HISTORY, KILL, HELP };

// FUNCTION PROTOTYPES FOR SHELL //
int get_history_size(char ** history_array);
char * get_past_history(char * command, char ** history_array);
int parse_int_arguments(char * args, int index_set);
void add_to_history(char * command, char ** history_array);
void remove_bg_process(int process_id);
void print_history(char ** history_array);
int kill_function(struct commandType * com);
void exit_function(void);
int help_function(void);
void jobs_function(void);
void remove_bg_process(int process_id);
int cd_function(parseInfo* info);
void pipeline(parseInfo * info, int index, int fd_in);
int execute_built_in_command(parseInfo * info, enum BUILTIN_COMMANDS, char ** history);
char * tilde_expansion_file (char * file);
char ** wildcard_expansion_file(char * file);

/* -----------------------------------------------------------------------------
FUNCTION: get_chararray_size(char ** arguments)
DESCRIPTION: will get the size of a string array (char *) array. Used in conjunction with
create_wildcard_remix to create wildcard.
-------------------------------------------------------------------------------*/
int get_chararray_size(char ** arguments)
{
    int size = 0;
    while (arguments[size] != NULL)
        size++;
    return size;
}

/* -----------------------------------------------------------------------------
FUNCTION: create_wildcard_remix(char ** arguments)
DESCRIPTION: uses glob() to parse through any possible use of "*" or wildcard to
expand on it within the shell. If there is not a wildcard "*" found within the arguments
then it will return NULL.
-------------------------------------------------------------------------------*/
char ** create_wildcard_remix(char ** arguments)
{
    int size = get_chararray_size(arguments);
    int index;
    char * wildcards[11];
    char * non_wild_cards[11];
    char * argument_copy;
    glob_t result;
    int error;
    int wildcard_index = 0;
    int non_wildcard_index = 0;
    for (index = 0; index < size; index++)
    {
        argument_copy = strdup(arguments[index]);   
        if (strstr(arguments[index], "*") != NULL)
        {
            wildcards[wildcard_index] = argument_copy;
            wildcard_index++;
        }
        else
        {
            non_wild_cards[non_wildcard_index] = argument_copy;
            non_wildcard_index++;
        }
    }
    if (wildcard_index == 0)
    {
        return NULL;
    }
    error = glob(wildcards[0], 0, NULL, &result);
    if (wildcard_index > 1)
    {
        for (index = 1; index < wildcard_index; index++)
        {
            error = glob(wildcards[index], GLOB_APPEND, NULL, &result);
        }
    }
    char ** new_wildcard_args = malloc((1 + non_wildcard_index + result.gl_pathc + 1) * sizeof(new_wildcard_args));
    int new_wildcard_index = 0;
    new_wildcard_args[new_wildcard_index++] = non_wild_cards[0];
    for (index = 1; index < non_wildcard_index; index++)
    {   
        new_wildcard_args[new_wildcard_index++] = non_wild_cards[index];
    }
    for (size_t  i = 0; i < result.gl_pathc; i++)
    {
        new_wildcard_args[i + new_wildcard_index] = result.gl_pathv[i];
    }
    new_wildcard_args[new_wildcard_index + result.gl_pathc] = NULL;
    return new_wildcard_args;
}

/* -----------------------------------------------------------------------------
FUNCTION: tilde_expansion_remix(parseInfo * info, int com_index)
DESCRIPTION: utilizes the information from the parse info as well as the index placed within to 
create a new char ** which will contain the appropriate arguments with expanded *
-------------------------------------------------------------------------------*/
char ** tilde_expansion_remix(parseInfo* info, int com_index)
{
    char * home_path = getenv("HOME");
    char * home_variable = "/home/myid/";
    struct commandType * com;
    com = &info->CommArray[com_index];
    char ** arguments = malloc(com->VarNum * sizeof(char*));
    int index;
    for (index = 0; index < com->VarNum; index++)
    {
        char * argument_copy = strdup(com->VarList[index]);
        if (argument_copy[0] == '~')
        {
            if (argument_copy[1] == '/')
            {
                if (strlen(argument_copy) == 2)
                {
                    argument_copy = strdup(home_path);
                    arguments[index] = argument_copy;
                } 
                else
                {
                    argument_copy ++;
                    char * home_path_include = strdup(home_path);
                    int new_length = strlen(home_path_include) + strlen(argument_copy) + 1;
                    char * finished = malloc(new_length * sizeof(char));
                    strcpy(finished, home_path_include);
                    strcat(finished, argument_copy);
                    arguments[index] = finished;
                }
            }
            else 
            {
                argument_copy++;
                char * home_var_include;
                home_var_include = strdup(home_variable);
                int new_length = strlen(home_var_include) + strlen(argument_copy) + 1;
                char * finished = malloc(new_length * sizeof(char));
                strcpy(finished, home_var_include);
                strcat(finished, argument_copy);
                arguments[index] = finished;
            }
        }
        else 
        {
            arguments[index] = argument_copy;
        }
    }
    arguments[com->VarNum] = NULL;
    return arguments;
}

/* -----------------------------------------------------------------------------
FUNCTION: tilde_expansion_file(char * file)
DESCRIPTION: will use tilde expansion for a char * (mostly meant for files)
-------------------------------------------------------------------------------*/
char * tilde_expansion_file (char * file)
{
    char * home_path = getenv("HOME");
    char * home_variable = "/home/myid/";
    char * argument_copy = strdup(file);
    if (argument_copy[0] == '~')
    {
        if (argument_copy[1] == '/')
        {
            if (strlen(argument_copy) == 2)
            {
                argument_copy = strdup(home_path);
                return argument_copy;
            } 
            else
            {
                argument_copy ++;
                char * home_path_include = strdup(home_path);
                int new_length = strlen(home_path_include) + strlen(argument_copy) + 1;
                char * finished = malloc(new_length * sizeof(char));
                strcpy(finished, home_path_include);
                strcat(finished, argument_copy);
                return finished;
            }
        }
        else 
        {
                argument_copy++;
                char * home_var_include;
                home_var_include = strdup(home_variable);
                int new_length = strlen(home_var_include) + strlen(argument_copy) + 1;
                char * finished = malloc(new_length * sizeof(char));
                strcpy(finished, home_var_include);
                strcat(finished, argument_copy);
                return finished;
            }
    }
    else 
    {
        return argument_copy;
    }
}
 
/* -----------------------------------------------------------------------------
FUNCTION: get_past_history(char * command, char ** history_array)
DESCRIPTION: will get the char * or string literal of past argument determined
based on the command (which is a number) and will call from array.
-------------------------------------------------------------------------------*/
char * get_past_history(char * command, char ** history_array)
{
    char * copy_history;
    if (command[1] == '-')
    {
        int offset = parse_int_arguments(command, 2);
        int size = get_history_size(history_array);
        if (offset > size)
        {
            copy_history = "error";
            return copy_history;
        }
        int last_index = size - offset;
        copy_history = strdup(history_array[last_index]);
        char * add_history = strdup(history_array[last_index]);
        add_to_history(add_history, history_array);
    }
    else
    {
        int last_index = parse_int_arguments(command, 1);
        int size = get_history_size(history_array);
        if (last_index > size)
        {
            copy_history = "error";
            return copy_history;
        }
        last_index--;
        copy_history = strdup(history_array[last_index]);
        char * add_history = strdup(history_array[last_index]);
        add_to_history(add_history, history_array);
    }
    return copy_history;
}

 
/* -----------------------------------------------------------------------------
FUNCTION: parse_int_arguments(char * args, int index_set)
DESCRIPTION: works together with get_past_history() to give the correct number to 
pass through to obtain the correct past_history call.
-------------------------------------------------------------------------------*/
int parse_int_arguments(char* args, int index_set)
{
    int num;
    args = args + index_set;
    num = atoi(args);
    return num;
}

 
/* -----------------------------------------------------------------------------
FUNCTION: get_history_size(char ** history_array)
DESCRIPTION: gets the size of the history_array. this function is used together with
other function calls.
-------------------------------------------------------------------------------*/
int get_history_size(char ** history_array)
{
    int size = 0;
    int index = 0;
    for (index = 0; index < 10; index++)
    {
        if (history_array[index] == NULL)
        {
            continue;
        }
        else
        {
            size++;
        }
    }
    return size;
}

 
/* -----------------------------------------------------------------------------
FUNCTION: add_to_history(char* command, char ** history_array)
DESCRIPTION: adds char * command to the history array determined by the history_array.
-------------------------------------------------------------------------------*/
void add_to_history(char * command, char ** history_array)
{
    int size = get_history_size(history_array);
    if (size < 10)
    {
        history_array[size] = command;
    }
    else if (size == 10)
    {
        int index;
        for (index = 0; index < 9; index++)
        {
            history_array[index] = history_array[index + 1];
        }
        history_array[9] = command;
    }
}

/* -----------------------------------------------------------------------------
FUNCTION: print_history(void)
DESCRIPTION: will print all history items within history.
-------------------------------------------------------------------------------*/
void print_history( char ** history_array)
{
    int index;
    int history_index = 1;
    for (index = 0; index < 10; index++) 
    {
        if (history_array[index] == NULL) 
        {
            continue;
        }
        else
        {
            printf("%d. %s\n", history_index, history_array[index]);
            fflush(stdout);
            history_index++;
        }
    }
}

 
/* -----------------------------------------------------------------------------
FUNCTION: help_function(void)
DESCRIPTION: prints help information to stdout
-------------------------------------------------------------------------------*/
int help_function()
{
    printf("::: HELPMENU :::\n");
    printf("A) BUILT-IN-FUNCTIONS\n");
    printf("     i) cd [directory name] - Change to another directory\n");
    printf("    ii) exit - quit shell unless process is running in bg\n");
    printf("    iii) jobs - output all processes that are currently running.\n");
    printf("    iv) history - will show the previous 10 commands in FIFO order.\n");
    printf("    v) ![int] - go back to this command via history/fifo.\n");
    printf("    vii) !-[int] - go backwards this many commands via history/fifo.\n");
    printf("    viii) KILL:\n");
    printf("        kill [process id] - Terminate process by process number\n");
    printf("        kill %[num] - Terminate process by order within JOBS process.\n");
    printf("    ix) help - help menu.\n");
    fflush(stdout);
    return 0;
}

 
/* -----------------------------------------------------------------------------
FUNCTION: jobs_function(void)
DESCRIPTION: prints all processes ids within jobs.
-------------------------------------------------------------------------------*/
void jobs_function()
{
    int index;
    for (index = 0; index < proceses; index++)
    {
        if (background_process_arguments[index] == NULL)
        {
            continue;
        }
        else
        {
            printf("[%d] %d RUNNING \t %s\n", process_number[index], background_process[index], background_process_arguments[index]);
            fflush(stdout);
        }
    }
}

 
/* -----------------------------------------------------------------------------
FUNCTION: cd_function(parseInfo * info)
DESCRIPTION: changes directory within shell.
-------------------------------------------------------------------------------*/
int cd_function(parseInfo * info)
{
    struct commandType * com;
    com = &info->CommArray[0];
    if (com->VarNum > 1)
    {
        wordexp_t exp_result;
        wordexp(com->VarList[1], &exp_result, 0);
        if (access(exp_result.we_wordv[0], F_OK)  == -1)
        {
            return -1;
        }
        chdir(exp_result.we_wordv[0]);
        wordfree(&exp_result);
    }
    else
    {
        char * home_path = getenv("HOME");
        chdir(home_path);
    }

    return 0;
}

 
/* -----------------------------------------------------------------------------
FUNCTION: execute_built_in_command(parseInfo * info, enum BUILTIN_COMMANDS cmd, char ** history)
DESCRIPTION: uses enum command to determine how to proceed in executing built in commmands
if no built in command has been called then will return 1 if the built in command
has been called then it will return 0. If there is any errors then it will return that errno number.
-------------------------------------------------------------------------------*/
int execute_built_in_command(parseInfo * info, enum BUILTIN_COMMANDS cmd, char ** history)
{
    struct commandType * com;
    int error;
    int exit_status;
    com = &info->CommArray[0];
    switch(cmd)
    {
        case EXIT:
            exit_function();
            exit_status = 0;
            break;
        case JOBS:
            jobs_function();
            exit_status = 0;
            break;
        case CD:
            error = cd_function(info);
            if (error == -1)
            {
                exit_status = 2;
            }
            else 
            {
                exit_status = 0;
            }
            break;
        case HISTORY:
            print_history(history);
            break;
        case KILL:
            error = kill_function(com);
            if (error == 3)
            {
                exit_status = 3;
            }
            else
            {
                exit_status = 0;
            }
            break;
        case HELP:
            error = help_function();
            exit_status = 0;
            break;
        case NO_SUCH_BUILTIN:
            exit_status = 1;
            break;
    }

    return exit_status;


}

/* -----------------------------------------------------------------------------
FUNCTION: check_process_id(int pid)
DESCRIPTION: gets the index for a given process pid, if the pid is not located within
the process numbers then will return -1.
-------------------------------------------------------------------------------*/
int check_process_id(int num)
{

    int index;
    int index_number = -1;
    for (index = 0; index < proceses; index++)
    {
        if (process_number[index] == num)
        {
            index_number = index;
        }
    }
    return index_number;
}
     
/* -----------------------------------------------------------------------------
FUNCTION: check_pid(int pid)
DESCRIPTION: determines whether or not the pid exists. if it exists it will return 0
if it doesn't exit it will return 1;
-------------------------------------------------------------------------------*/
int check_pid(int pid)
{
    int index;
    int exit_status = 1;
    for (index = 0; index < proceses; index++)
    {
        if (background_process[index] == pid)
        {
            exit_status = 0;
        }

    }
    return exit_status;
}

/* -----------------------------------------------------------------------------
FUNCTION: get_pid(int pid)
DESCRIPTION: gets the pid from background_processes.
-------------------------------------------------------------------------------*/
int get_pid(int index)
{
    return background_process[index];
}

/* -----------------------------------------------------------------------------
FUNCTION: kill_function(struct commandType * com)
DESCRIPTION: will kill the process used to kill possibly background or lingering processes.
-------------------------------------------------------------------------------*/

int kill_function(struct commandType * com)
{
    int pid;
    int error;
    int exit_status;
    if (com->VarList[1][0] == '%')
    {
        char * copy = strdup(com->VarList[1]);
        copy++;
        int process_job_number = atoi(copy);
        int process_id_number = check_process_id(process_job_number);
        if (process_id_number < 0)
        {
            return 3;
        }
        pid = get_pid(process_id_number);
    }
    else
    {
        pid = atoi(com->VarList[1]);
    }
    error = check_pid(pid);
    if (error == 0)
    {
        int index = check_process_id(pid);
        kill(pid, SIGKILL);
        exit_status = 0;
    }
    else
    {
        fprintf(stderr, "PROCESS [%d] ", pid);
        exit_status = 3;
    }
    return exit_status;
}

/* -----------------------------------------------------------------------------
FUNCTION: exit_function(void)
DESCRIPTION: will exit the shell unless there is a process remaining then will print out
the remaining process.
-------------------------------------------------------------------------------*/

void exit_function()
{
    if (proceses > 0)
    {
        printf("CANNOT EXIT: current %d running processes\n", proceses);
        fflush(stdout);
    }
    else
    {
        exit(1);
    }
}

char ** wildcard_expansion_file(char * file)
{
    if (strstr(file, "*") != NULL)
    {
        glob_t result;
        int error;
        int index;
        glob(file, 0, NULL, &result);
        char ** file_wildcards = malloc(result.gl_pathc * sizeof(file_wildcards));
        for (index = 0; index < result.gl_pathc; index++)
        {
            file_wildcards[index] = result.gl_pathv[index];
        }
        file_wildcards[result.gl_pathc] = NULL;
        return file_wildcards;
    }
    else
    {
        return NULL;
    }
}
     
/* -----------------------------------------------------------------------------
FUNCTION: remove_bg_process(int process_id)
DESCRIPTION: this process works within signal handler to eliminate the proess within
the background_process array.
-------------------------------------------------------------------------------*/
void remove_bg_process(int process_id)
{
    int index;
    int index_remove = 0;
    for (index = 0; index < proceses; index++)
    {
        if (background_process[index] == process_id)
        {
            index_remove = index;
        }
    }
    proceses--;
    if (proceses == 0)
    {
        process_number_id = 1;
    }
    for (index = index_remove; index < proceses; index++)
    {   
        background_process[index] = background_process[index + 1];
        background_process_arguments[index] = background_process_arguments[index + 1];
        process_number[index] = process_number[index + 1];
    }
}

     
/* -----------------------------------------------------------------------------
FUNCTION: handle_sigchld(int s)
DESCRIPTION: signal call to work with SIGCHLD
-------------------------------------------------------------------------------*/
void handle_sigchld( int s)
{
    pid_t pid;
    int bool_terminated;
    while((pid = waitpid(0, NULL, WNOHANG)) > 0) 
    {
        remove_bg_process(pid);
    }
}

     
/* -----------------------------------------------------------------------------
FUNCTION: pipeline(parseInfo * info, int index, int fd_in)
DESCRIPTION: recursively will work and apply the pipes to all functions within arguments.
-------------------------------------------------------------------------------*/
void pipeline(parseInfo * info, int index, int fd_in)
{
    static int first_time = 0;
    struct commandType* com;
    com = &(info->CommArray[index + 1]);
    if ( ( com == NULL) || (com->command == NULL))
    {
        first_time = 0;
        int error;
        com = &(info->CommArray[index]);
        dup2(fd_in, STDIN_FILENO);
        if (info->boolOutfile == 1)
        {
            char * outFile = tilde_expansion_file(info->outFile);
            int new_fd = open(outFile, O_WRONLY | O_EXCL | O_CREAT, 0644);
            if (new_fd == -1)
            {
                if (errno == EEXIST)
                {
                    perror("");
                    exit(1);
                }
            }
            dup2(new_fd, 1);
        }
        char ** arguments_one = tilde_expansion_remix(info, index);
        char ** arguments_final = create_wildcard_remix(arguments_one);
        if (arguments_final != NULL)
        {
            error = execvp(arguments_final[0], arguments_final);
            if (error < 0)
            {
                printf("YOSH: [%s] not found\n", com->command);
                fflush(stdout);
                exit(1);
            }
        }
        else
        {    
            error = execvp(arguments_one[0], arguments_one); 
            if (error < 0)
            {
                printf("YOSH: [%s] not found\n", com->command);
                fflush(stdout);
                exit(1);
            }
        }
    }
    else 
    {
        com = &(info->CommArray[index]);
        int fd[2];
        pid_t childPid;
        pipe(fd);
        childPid = fork();
        if (childPid == 0)
        {
            int error;
            if ((info->boolInfile != 1) && (first_time == 0))
            {
                close(fd[0]);
            }
            dup2(fd_in, STDIN_FILENO);
            dup2(fd[1], 1);
            char ** arguments_one = tilde_expansion_remix(info, index);
            char ** arguments_final = create_wildcard_remix(arguments_one);
            if (arguments_final != NULL)
            {
                error = execvp(arguments_final[0], arguments_final);
                if (error < 0)
                {
                    dup2(std_out_fd, 1);
                    printf("YOSH: [%s] not found\n", com->command);
                    fflush(stdout);
                    exit(1);
                }
            }
            else
            {    
                error = execvp(arguments_one[0], arguments_one); 
                if (error < 0)
                {
                    dup2(std_out_fd, 1);
                    printf("YOSH: [%s] not found\n", com->command);
                    fflush(stdout);
                    exit(1);
                }
            }
        }
        else
        {
            first_time++;
            close(fd[1]);
            close(fd_in);
            pipeline(info, index + 1, fd[0]);
        }
    }
}

/* -----------------------------------------------------------------------------}
FUNCTION: buildPrompt()
DESCRIPTION: Creates prompt used within the shell
-------------------------------------------------------------------------------*/

char * buildPrompt()
{
    char * full_prompt;
    char full_name[PATH_MAX] = "YOSH:{";
    full_prompt = full_name;
    char* prompt;
    char home_path[PATH_MAX];
    prompt = home_path;
    getcwd(home_path, PATH_MAX);
    strcat(prompt, "}$ ");
    strcat(full_prompt, prompt);
    return full_prompt;
}
 
/* -----------------------------------------------------------------------------
FUNCTION: isBuiltInCommand();
DESCRIPTION: gets the argument and returns ENUM if matches.
-------------------------------------------------------------------------------*/
int isBuiltInCommand( char * cmd )
{

  if( strncmp(cmd, "exit", strlen( "exit" ) ) == 0 )
	{
	return EXIT;
  	}

  if ( strncmp(cmd, "jobs", strlen( "jobs" )) == 0)
  {
      return JOBS;
  }

  if (strncmp(cmd, "cd", strlen("cd")) == 0)
  {
      return CD;
  }
  
  if (strncmp(cmd, "history", strlen("history")) == 0)
  {
      return HISTORY;
  }

  if (strncmp(cmd, "kill", strlen("kill")) == 0)
  {
      return KILL;
  }

  if (strncmp(cmd, "help", strlen("help")) == 0)
  {
      return HELP;
  }
  return NO_SUCH_BUILTIN;
}

/* -----------------------------------------------------------------------------
FUNCTION: setup_info_with_pipes(parseInfo * info)
DESCRIPTION: assigns the appropriate signal handlers and input redirection to processes.
-------------------------------------------------------------------------------*/
int setup_info_with_pipes(parseInfo * info)
{
    if (info->pipeNum > 0)
    {
        signal(SIGCHLD, SIG_IGN);
    }

    if (info->boolInfile)
    {
        char * inFile = tilde_expansion_file(info->inFile);
        int new_fd = open(inFile, O_RDONLY);
        if (new_fd == -1)
        {
            if (errno == ENOENT)
            {
                return 2;
            }
        }
        dup2(new_fd, 0);
    }

    if (info->boolBackground)
    {
        signal(SIGCHLD, handle_sigchld);
    }
    return 0;
}
/* -----------------------------------------------------------------------------
FUNCTION: setup_info(parseInfo * info)
DESCRIPTION: creates pipes, in and output for processes
-------------------------------------------------------------------------------*/
int setup_info(parseInfo * info)
{
    if (info->pipeNum > 0)
    {
        signal(SIGCHLD, SIG_IGN);
    }

    if (info->boolInfile)
    {
        char * inFile = tilde_expansion_file(info->inFile);
        int new_fd = open(inFile, O_RDONLY);
        if (new_fd == -1)
        {
            if (errno == ENOENT)
            {
                return 2;
            }
        }
        dup2(new_fd, 0);
    }
    if (info->boolOutfile)
    {
        char * outFile = tilde_expansion_file(info->outFile);
        int new_fd = open(outFile, O_WRONLY | O_EXCL | O_CREAT, 0644);
        if (new_fd == -1)
        {
            if (errno == EEXIST)
            {
                return 17;
            }
        }
        dup2(new_fd,1);
    }

    if (info->boolBackground)
    {
        signal(SIGCHLD, handle_sigchld);
    }
    return 0;
}


/* -----------------------------------------------------------------------------
FUNCTION: main()
DESCRIPTION: main driver for the SHELL program.
-------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    enum BUILTIN_COMMANDS command_check;
    char * prompt;
  char * cmdLine;
  parseInfo *info; 		// info stores all the information returned by parser.
  struct commandType *com; 	// com stores command name and Arg list for one command.
  char* history[10] = { NULL };
  char* command_history;
  char terminated_process[81];
    background_process = (int*) calloc(process_size, sizeof(int));
    background_process_arguments = malloc(process_size * sizeof(char*));
    process_number = (int*) calloc(process_size, sizeof(int));
    int i;
    for (i = 0; i < process_size; i++)
    {
        background_process_arguments = malloc(81 * sizeof(char));
    }
  int error;
  std_out_fd = dup(1);
  std_in_fd = dup(0);

  fprintf( stdout, "This is the YOSH shell by Christopher Lee\n" ) ;
  fflush(stdout);
    
  while(1)
  {
        //create prompt
        prompt = buildPrompt();
        
        //read line from cmdline
    	cmdLine = readline( prompt ) ;
        
    	if( cmdLine == NULL ) 
	    {
      		fprintf(stderr, "Unable to read command\n");
      		continue;
    	}
        
        //creates copy of cmdLine for use in functions and error checking.
        command_history = strdup(cmdLine);
        
        if ((*cmdLine != '\n') && (*cmdLine != '\0'))
        {
            // execute the past history action
            if (command_history[0] == '!')
            {
                char * error_char;
                error_char = get_past_history(command_history, history);
                if (strcmp(error_char, "error") == 0) // detected error within history 
                {
                    errno = EPERM;
                    perror("invalid history call");
                    continue;
                }
                cmdLine = error_char;
            }
            //add to history
            else
            {
                add_to_history(command_history, history);
            }
        } 
        
        //create parseInfo struct
    	info = parse( cmdLine );
        if( info == NULL )
    	{
      		free(cmdLine);
      		continue;
    	}
        
        // no pipes were found.
        if (info->pipeNum == 0)
        {
            //set up signals, input & output fd, etc.
            int error = setup_info(info);
            if (error > 0)
            {
                perror("");
                free_info(info);
                free(cmdLine);
                continue;
            }

    	    com = &info->CommArray[0];
    	    if( (com == NULL)  || (com->command == NULL)) 
    	    {
      		    free_info(info);
      		    free(cmdLine);
      		    continue;
    	    }
        
            //check for built in commands
            command_check = isBuiltInCommand(com->command);
            error = execute_built_in_command(info, command_check, history);
        
            // successful built in command
            if (error == 0)
            {   
                dup2(std_out_fd, 1);
                dup2(std_in_fd, 0);
                free(cmdLine);
                free_info(info);
                continue;
            }

            // error within directory
            if (error == 2)
            {
                fprintf(stderr, "Directory: %s ", com->VarList[1]);
                perror("");
      		    free_info(info);
      	        free(cmdLine);
                continue;
            }
            // process does not exist
            if (error == 3)
            {
                errno = ESRCH;
                perror("");
                free_info(info);
                free(cmdLine);
                continue;
            }          

            //proceed to execute commands if built in command not completed.
            pid_t childPid;
            int status;
            childPid = fork();
            if (childPid == 0) //child process
            {   
                // tilde expansion is utilized
                char ** arguments_one = tilde_expansion_remix(info, 0);
                // wildcard is utilized
                char ** arguments_final = create_wildcard_remix(arguments_one);
                if (arguments_final != NULL) // wild card is found.
                {
                    error = execvp(arguments_final[0], arguments_final);
                    if (error < 0)
                    {
                        printf("YOSH: [%s] not found\n", com->command);
                        exit(1);
                    }
                }
                else // wildcard is not found.
                {
                    error = execvp(arguments_one[0], arguments_one);
                    if (error < 0)
                    {
                        printf("YOSH: [%s] not found\n", com->command);
                        exit(1);
                    }
                }
            }
            else  // parent process
            {
                // is a background process
                if (info->boolBackground) 
                {   
                    //add information to background arrays
                    int copy_pid = (int) childPid;
                    process_number[proceses] = process_number_id;
                    background_process[proceses] = copy_pid;
                    background_process_arguments[proceses] = command_history;
                    printf("[%d] %d\n", process_number_id, copy_pid);
                    fflush(stdout);
                    process_number_id++;
                    proceses++;
                    if (proceses == process_size - 1) // processes are becoming too large
                    {
                        //expand size for all arrays
                        process_size = process_size * 2;
                        background_process = (int*) realloc(background_process, process_size * sizeof(int));
                        process_number = (int *) realloc(process_number, process_size * sizeof(int));
                        background_process_arguments = realloc(background_process_arguments, process_size * sizeof(*background_process_arguments));
                    }
                }
                // is not a background process.
                else
                {
                    waitpid(childPid, &status, 0);
                }
            }

        }
        // pipes are greater than 0
        else
        {
            // set up signals and input and output.
            int error = setup_info_with_pipes(info);
            // error with input or setup
            if (error > 0)
            {
                perror("");
                free_info(info);
                free(cmdLine);
                continue;
            }

    	    com = &info->CommArray[0];
    	    if( (com == NULL)  || (com->command == NULL)) 
    	    {
      		    free_info(info);
      		    free(cmdLine);
      		    continue;
    	    }
            //check for builtin commands and execute them.
            command_check = isBuiltInCommand(com->command);
            error = execute_built_in_command(info, command_check, history);
        
            // successful built in command
            if (error == 0)
            {   
                dup2(std_out_fd, 1);
                dup2(std_in_fd, 0);
                free(cmdLine);
                free_info(info);
                continue;
            }

            // error within directory
            if (error == 2)
            {
                fprintf(stderr, "Directory: %s ", com->VarList[1]);
                perror("");
      		    free_info(info);
      	        free(cmdLine);
                continue;
            }
            
            //process not found.
            if (error == 3)
            {
                errno = ESRCH;
                perror("");
                free_info(info);
                free(cmdLine);
                continue;
            }
            //check if process is meant to be a signal.
            if (info->boolBackground)
            {
                signal(SIGCHLD, handle_sigchld);
            }

    	    com = &info->CommArray[0];
    	    if( (com == NULL)  || (com->command == NULL)) 
    	    {
      		    free_info(info);
      		    free(cmdLine);
      		    continue;
    	    }
            pid_t childPid;
            int status;
            childPid = fork();
            if (childPid == 0) // child process
            {   
                pipeline(info, 0, 0); // recursive call to piping...
            }
            else  // parent process
            {
                // backgroud process.
                if (info->boolBackground)
                {   
                    // add to background arrays
                    int copy_pid = (int) childPid;
                    process_number[proceses] = process_number_id;
                    background_process[proceses] = copy_pid;
                    background_process_arguments[proceses] = command_history;
                    printf("[%d] %d\n", process_number_id, copy_pid);
                    fflush(stdout);
                    process_number_id++;
                    proceses++;
                    if (proceses == process_size - 1) // process size is needs to increase in arrays
                    {
                        //increase size in all arrays.
                        process_size = process_size * 2;
                        background_process = (int*) realloc(background_process, process_size * sizeof(int));
                        process_number = (int *) realloc(process_number, process_size * sizeof(int));
                        background_process_arguments = realloc(background_process_arguments, process_size * sizeof(*background_process_arguments));
                    }
                }
                //is not a background process
                else 
                {
                    waitpid(childPid, &status, 0);
                }
            }
        }
    

    // restore any fd back to original.
    dup2(std_out_fd, 1);
    dup2(std_in_fd, 0);
    free_info(info);
    free(cmdLine);
    

  }/* while(1) */
  free(prompt);
  free(command_history);
  free(background_process);
  free(background_process_arguments);

}
  





