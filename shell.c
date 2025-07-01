#include<stdlib.h>
#include<io.h>
#include<stdio.h>
#include<stdbool.h>
#include<sys/types.h>
#include<string.h>
#include<unistd.h>
#include <sys/types.h>
// #include <sys/wait.h>

#define SHL_RL_BUFFER_SIZE 1024  //bytes
#define SHL_TOK_BUFFER_SIZE 64 //buff size for single token
#define SHL_TOK_DELIMITERS " \t\n\a\r" 

/*Prototypes*/

//Built-in shell commands

int shl_cd (char **args);
int shl_help (char **args);
int shl_exit (char **args);


//shell processes
char * shl_read_line ();
char ** shl_parse_line (char* line);
int shl_launch (char ** args);

/*----*/

//read the line command
char * shl_read_line () 
{
    int buffer_size = SHL_RL_BUFFER_SIZE;
    int pos = 0;
    char * buffer = malloc (buffer_size * sizeof(char)) ;
    int c; //not char - to detect EOF, EOF is an int
    
    //exceed size
    if (!buffer) 
    {
        fprintf (stderr, "shl: allocation error\n");
        exit (1);
    }

    while (true) 
    {
        c = getchar();
        if (c==EOF || c=='\n') 
        {
            buffer[pos]='\0';
            return buffer;
        } 
        else 
        {
            buffer[pos]=c;
        }
        pos++;

        if (pos>= buffer_size)
        {
            buffer_size += SHL_RL_BUFFER_SIZE;
            buffer = realloc (buffer, buffer_size);
            if (!buffer) 
            {
                fprintf (stderr, "shl: allocation error\n");
                exit (EXIT_FAILURE);
            }
        }
    }
}

//reading with getline using stdio.h
char * shl_read_line_o ()
{
    char * line = NULL;
    ssize_t buffer_size = 0;


    if (getline (&line, &buffer_size, stdin) == -1) 
    {
        if (feof(stdin)) 
        {
            exit(EXIT_SUCCESS);
        }
        else 
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }

    return line;
}


//parsing args   delimiter-spaces

char ** shl_parse_line (char * line) {
    int buffer_size = SHL_TOK_BUFFER_SIZE;
    int pos = 0;
    char **tokens = malloc (buffer_size * sizeof(char));
    char * token;

    if (!tokens)
    {
        fprintf(stderr, "shl: alloc error\n");
        exit (EXIT_FAILURE);
    }

    //see documentation about strtok replaces delim with '\0'

    token = strtok(line, SHL_TOK_DELIMITERS);//for the first token
    

    while (token != NULL)
    {
        tokens[pos] = token;
        pos++;

        if (pos >= buffer_size)
        {
            buffer_size+=SHL_TOK_BUFFER_SIZE;
            tokens = realloc (tokens, buffer_size * sizeof(char));
            if (!tokens)
            {
                fprintf(stderr, "shl: alloc error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHL_TOK_DELIMITERS);
    }

    tokens[pos] = NULL;
    
    return tokens;
}


//for launching third party apps from CLI
int shl_launch (char ** args) {
    pid_t pid, wpid;

    int status;

    pid = fork();
    if (pid == 0)   //PID 0 is used internally by the kernel
    {
        //child process
        if (execvp(args[0], args) == -1) //run user command
        {
            perror("shl");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0)  //processes don't have negative PIDs
    {
        perror("shl");
    } else {
        //Parent process
        do {
            // wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


//list of Built-in commands

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

//store functions in a array of ptrs
int (*builtin_func[]) (char **) = {
    &shl_cd,
    &shl_help,
    &shl_exit
};

int shl_num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}

/*Built in implementaions*/

int shl_cd (char ** args) 
{
    if (args[1] == NULL) {
        fprintf(stderr, "shl: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0 ){
            perror("shl");
        }
    }
    return 1;
}


int shl_help (char ** args) 
{
    printf("SHELL 1.0\n");
    printf("Built-in functions: \n");

    for(int i =0; i < shl_num_builtins(); i++)
    {
        printf(" %s\n", builtin_str[i]);
    }

    return 1;
}

int shl_exit (char **args) 
{
    return 0;
}

int shl_execute (char ** args) {
    
    if (args[0] == NULL)  //empty command
    {
        return 1;
    }

    for (int i = 0; i < shl_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return shl_launch(args);
    
}


void shl_loop () 
{
    char * line;
    char ** args;

    int status;

    do 
    {
        printf(">> ");
        line = shl_read_line ();
        args = shl_parse_line (line);
        status = shl_execute (args);

        free (line);
        free(args);
    } while (status);
}


int main (int argc, char ** argv) 
{

    shl_loop();

    return 0;
}

