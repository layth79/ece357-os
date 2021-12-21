#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <sys/resource.h>
#include <signal.h> 
#include <sys/wait.h>

int child(char *line[], int idx);

int main(int argc, char *argv[])
{
    int exitVal = 0;
    char lineBuffer[4096];
    char *token;
    char *line[4096];
    FILE *stream;
    
    // checks if a script is provided as an argument
    if (argc > 2)
    {
        fprintf(stderr,"ERROR: Incorrect usage");
    } 
    else if (argc == 2 && (stream = fopen(argv[1],"r")) == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file to read: %s.\n", strerror(errno));
    }
    else if (argc == 1)
    {
        stream = stdin; 
    }

    fseek(stream, 0, SEEK_END); 
    int file_size = ftell(stream); 
    fseek(stream, 0, SEEK_SET); 
    
    // each iteration gets a line of input (i.e., command with args and redirections)
    while (fgets(lineBuffer, 4096, stream))
    {
        if (ftell(stream) > file_size)
        {
            fclose(stream); 
            exit(exitVal);
        }
        // checks for comment or a single newline char
        if (lineBuffer[0] == '#' || lineBuffer[0] == '\n')
        {
            continue;
        }

        // tokenize command
        token = strtok(lineBuffer, " \n");
        line[0] = token;
        int idx = 1;
        while (token != NULL)
        {
            token = strtok(NULL, " \n");
            line[idx] = token;
            ++idx;
        }
        --idx;  
        
        // executes built-in pwd command
        if (!strcmp(line[0], "pwd"))
        {
            char s[1024];
            if (getcwd(s, 1024) != NULL)
            {
                printf("%s\n", s);
            }
            else
            {
                fprintf(stderr, "ERROR: %s when calling pwd.\n", strerror(errno));
            }
        }
        // executes built-in cd command
        else if (!strcmp(line[0], "cd"))
        {
            char* destDir;
            if(idx == 1)
            {
                destDir = getenv("HOME");
            }
            else
            {
                destDir = line[1];
            }
            if (chdir(destDir) < 0)
            {
                fprintf(stderr, "ERROR: cannot go to %s: %s\n", destDir, strerror(errno));
            }
        }
        // executes built-in exit command
        else if(!strcmp(line[0], "exit"))
        {
            if(idx == 1)
            {
                exit(exitVal);
            }
            else
            {
                exit(atoi(line[1]));
            }
        }
        // executes non-built-in command
        else 
        {
            pid_t pid;
            int wstatus; 
            float realtime;
            
            struct rusage rusage;
            struct timeval start;
            struct timeval end;

            // start of process time
            if (gettimeofday(&start, NULL) < 0)
            {
                fprintf(stderr, "ERROR: gettimeofday failed: %s \n", strerror (errno));
            } 
            int ret; 
            switch (pid = fork())
            {
                // executes child process resulting from fork
                case 0:
                    child(line, idx);
                    break;
                // exits if fork sys call fails
                case -1: 
                    fprintf(stderr, "ERROR: command %s: %s\n", line[0], strerror(errno));
                    exit(EXIT_FAILURE);
                default:
                    // parent process waits for child process to exit
                    if (wait3(&wstatus, 0, &rusage) == -1)
                    {
                        fprintf(stderr, "ERROR: wait failed: %s \n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                    // checks if status returned for child process terminated normally
                    if (WIFEXITED(wstatus))
                    {
                        exitVal = WEXITSTATUS(wstatus);
                        fprintf(stderr, "Child process %d exited with value %d\n", pid, exitVal);
                    }
                    // checks if status returned for child process terminated because of a signal
                    else if (WIFSIGNALED(wstatus))
                    {
                        exitVal = WTERMSIG(wstatus); 
                        fprintf(stderr, "Child process %d exited with signal %d (%s)\n", pid, exitVal, strsignal(exitVal));
                    }

                    // end of process time
                    if (gettimeofday(&end, NULL) < 0)
                    {
                        fprintf( stderr, "ERROR: with gettimeofday: %s \n", strerror (errno));
                    } 

                    // outputs process information
                    fprintf(stderr, "Real: %.3fs ", (double) (end.tv_usec - start.tv_usec)/1000000); 
                    fprintf(stderr, "User: %.3fs ", (double) rusage.ru_utime.tv_usec/1000000);
                    fprintf(stderr, "Sys: %.3fs\n", (double) rusage.ru_stime.tv_usec/1000000);
                    break;
            }
        }
    }
}

// redirects if the command requires it
int redirectshun(int mode, char *file, int fd2)
{
    int fd;
    if ((fd = open(file, mode, 0666)) < 0)
    {
        fprintf(stderr, "ERROR: Cannot open %s: %s\n", file, strerror(errno));
        return 69;
    }
    if (dup2(fd, fd2) < 0)
    {
        fprintf(stderr, "ERROR: dup2: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(fd) < 0)
    {
        fprintf(stderr,"ERROR: while closing %s : %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int child(char *line[], int idx)
{
    int redirects = 0;
    char *file;
    int ret;

    for(int i = 0; i < idx; i++)
    {
        if(line[i][0] == '<')
        {
            // Open filename and redirect stdin
            file = line[i] + 1;
            ret = redirectshun(O_RDONLY, file, 0);
            if (ret == 69)
            {
                exit(EXIT_FAILURE); 
            }
            ++redirects;
        }
        else if((line[i][0]) == '>')
        {
            if((line[i][1]) == '>')
            {
                // Open/Create/Append filename and redirect stdout
                file = line[i] + 2;
                ret = redirectshun(O_WRONLY|O_CREAT|O_APPEND, file, 1);
                if (ret == 69)
                {
                    exit(EXIT_FAILURE); 
                }
                ++redirects;
            }
            else
            {
                // Open/Create/Truncate filename and redirect stdout
                file = line[i] + 1;
                ret = redirectshun(O_WRONLY|O_CREAT|O_TRUNC, file, 1);
                if (ret == 69)
                {
                    exit(EXIT_FAILURE); 
                }
                ++redirects;
            }
        }
        else if(line[i][0] == '2' && line[i][1] == '>')
        {
            if((line[i][2]) == '>')
            {
                // Open/Create/Append filename and redirect stderr
                file = line[i] + 2;
                ret = redirectshun(O_WRONLY|O_CREAT|O_APPEND, file, 2);
                if (ret == 69)
                {
                    exit(EXIT_FAILURE); 
                } 
                
                ++redirects;
            }
            else
            {
                // Open/Create/Truncate filename and redirect stderr
                file = line[i] + 3;
                ret = redirectshun(O_WRONLY|O_CREAT|O_TRUNC, file, 2);
                if (ret == 69)
                {
                    exit(EXIT_FAILURE); 
                } 
                ++redirects;
            }
        } 
    }
    int length = idx - redirects;
    line[length] = NULL;
    execvp(line[0], line);
    fprintf(stderr,"ERROR: fail to exec %s: %s\n", line[0], strerror(errno));
    exit(127);
}
