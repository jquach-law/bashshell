#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Global variables
int MAX_CHAR = 2049;
int MAX_ARGU = 513;

// setting to ignore or not ignore background commands
char ignore_bg = NULL;

// signal function: entering and exiting foreground-only mode
void ignore_bgcommands(int sig)
{
    // exiting foreground mode
    if (ignore_bg)
    {
        printf("EXITING foreground-only mode\n");
        ignore_bg = NULL;
    }

    // entering foreground mode
    else
    {
        printf("Entering foreground-only mode (ignoring &)\n");
        ignore_bg = 'T';
    }
}

// execution main
int main()
{
    // SIGINT from Ctrl+C
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, ignore_bgcommands);

    // main() variables
    char *currDir;
    int childStatus;
    int childPid;

    // array to hold background PID
    int arrPID[100] = {};

    // beginning: ask for input
    char input[2049];

    // while input isnt NULL
    while (input)
    {
        // looking for background terminated children
        for (int i = 0; i < 50; i++)
        {
            // if element isnt empty
            if (arrPID[i])
            {
                // process child
                childPid = waitpid(arrPID[i], &childStatus, WNOHANG);
                if (childPid)
                {
                    // if child exited normally
                    if (WIFEXITED(childStatus))
                    {
                        printf("BACKGROUND PID %d is done: exit value %d\n", childPid, WEXITSTATUS(childStatus));
                    }

                    // if child was terminated
                    if (WIFSIGNALED(childStatus))
                    {
                        printf("BACKGROUND PID %d is terminated by sigal: %d\n", childPid, WTERMSIG(childStatus));
                    }

                    // clear processed child from array
                    arrPID[i] = NULL;
                }
            }
        }

        // ask for input
        printf(": ");
        fflush(stdout);
        fgets(input, MAX_CHAR, stdin);
        fflush(stdin);

        // ignore lines start with '#' and '\n'
        if (input[0] == '\n' || input[0] == '#')
        {
            // re-ask for input
            continue;
        }

        // if character len shorter than 2049
        if (strlen(input) <= MAX_CHAR)
        {

            // convert any $$ in command
            const char needle2[4] = "$$";

            // find first occurence of "$$"
            char *locptr2 = strstr(input, needle2);
            while (locptr2)
            {
                char newstring[1024];

                // move string pointer by 2 for $$
                strcpy(newstring, locptr2 + 2);

                // replacing $$ with PID at pointer
                char pid[30];
                sprintf(pid, "%d", getpid());
                strcpy(locptr2, pid);

                // append back saved string
                strcat(input, newstring);

                // check rest of string
                locptr2 = strstr(input, needle2);
            }

            // parse command
            char *token = strtok(input, " \n");

            // count number of arguments (max: 512)
            int count = 0;

            // three built-in commands
            if (token)
            {
                // if command is exit
                if (strcmp(token, "exit") == 0)
                {
                    // terminate all child processes
                    for (int i = 0; i < 50; i++)
                    {
                        if (arrPID[i])
                            kill(arrPID[i], SIGTERM);
                    }

                    // parent exiting itself
                    exit(0);
                }
                // if command is change directory
                else if (strcmp(token, "cd") == 0)
                {
                    // taking command argument
                    token = strtok(NULL, " \n");
                    if (token)
                    {
                        // path error
                        if (chdir(token) != 0)
                            perror("Invalid Path");
                    }

                    // if just "cd", return HOME
                    else
                    {
                        chdir(getenv("HOME"));
                    }

                    // re-ask for input
                    continue;
                }

                // if ask for exit/terminate status
                else if (strcmp(token, "status") == 0)
                {
                    // return default or previous child status
                    printf("exit status %d\n", WEXITSTATUS(childStatus));

                    // re-ask for input
                    continue;
                }
            }

            // catenating string for execv() command
            int index = 0;
            int *newargv[MAX_ARGU];
            char command[] = "/bin/";
            strcat(command, token);

            // first element of execv() command
            newargv[index] = command;

            // taking/counting additional arguments for execv() command
            token = strtok(NULL, " \n");
            count++;

            // initializing I/O file for redirections
            char *input_file = NULL;
            char *output_file = NULL;

            // while there's additional argument
            while (token)
            {
                // check if input symbol '<'
                if (strcmp(token, "<") == 0)
                {
                    // look for input file
                    token = strtok(NULL, " \n");
                    count++;

                    // remember input file
                    if (token)
                        input_file = token;
                }

                // check if output symbol '>'
                else if (strcmp(token, ">") == 0)
                {
                    // look for output file
                    token = strtok(NULL, " \n");
                    count++;

                    // remember output file
                    if (token)
                        output_file = token;
                }

                // else take as regular argument
                else
                {
                    index++;
                    newargv[index] = token;
                }

                // next token string to process
                token = strtok(NULL, " \n");
                count++;
            }

            // checking for max argument of 512
            if (count > MAX_ARGU)
            {
                printf("too many arguments");
                continue;
            }

            // noting background command, replacing & with NULL

            // default background value to NULL
            char runback = NULL;

            // if last argument is '&'
            if (strcmp(newargv[index], "&") == 0)
            {
                // regular background execution: runback as True
                if (!ignore_bg)
                {
                    runback = 'T';
                }
                // FOREGROUND-ONLY mode: leave 'runback' as NULL

                // puting NULL at end of exec()
                newargv[index] = NULL;
            }

            // run as regular foreground command
            else
            {
                index++;
                newargv[index] = NULL;
            }

            // forking
            pid_t spawnpid = fork();

            // parent and children processes
            switch (spawnpid)
            {
            case -1:
                // fork error
                perror("fork() failed");
                exit(1);
                break;

            // successful fork: CHILD PROCESS
            case 0:
                // child to ignore signal SIGTSTP
                signal(SIGTSTP, SIG_IGN);

                // if run as background, ignore signal SIGINT
                if (runback)
                    signal(SIGINT, SIG_IGN);

                // if run as backgroud, and no input file provided
                if (runback && !input_file)
                {
                    // default input from /dev/null
                    int sourceFD = open("/dev/null", O_RDONLY);
                    dup2(sourceFD, 0);
                }

                // if runback and no output file given
                if (runback && !output_file)
                {
                    // default output to /dev/null
                    int targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    dup2(targetFD, 1);
                }

                // if redirect input
                if (input_file)
                {
                    // open input file as read only
                    int sourceFD = open(input_file, O_RDONLY);

                    // open error
                    if (sourceFD == -1)
                    {
                        perror("redirect input");
                        exit(1);
                    }

                    // Redirect stdin from input file
                    int result = dup2(sourceFD, 0);

                    // redirect error
                    if (result == -1)
                    {
                        perror("source dup2()");
                        exit(2);
                    }
                }

                // if output redirect
                if (output_file)
                {
                    // Open target file: write, truncate, create
                    int targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                    // Open file error
                    if (targetFD == -1)
                    {
                        perror("redirect output");
                        exit(1);
                    }

                    // Redirect stdout to target file
                    int result = dup2(targetFD, 1);

                    // redirect error
                    if (result == -1)
                    {
                        perror("target dup2()");
                        exit(2);
                    }
                }

                // Replace the current program with execv() command
                execv(newargv[0], newargv);

                // exec only returns if there is an error
                perror("execve");

                // end process
                exit(0);

            // PARENT PROCESS
            default:
                // BACKGROUND CHILD PROCESS
                //  remember background child in a PID array
                if (runback)
                {
                    // dont wait for background child
                    childPid = waitpid(spawnpid, &childStatus, WNOHANG);

                    // find NULL spot in PID array
                    for (int i = 0; i < 25; i++)
                    {
                        if (arrPID[i] == NULL)
                        {
                            // send message: backgroud child PID
                            printf("background pid is %d\n", spawnpid);

                            // remember backgroud child
                            arrPID[i] = spawnpid;
                            break;
                        }
                    }
                }
                // FOREGROUND CHILD PROCESS
                //  else wait for foreground child process
                else
                {
                    // wait for foreground child
                    childPid = waitpid(spawnpid, &childStatus, 0);

                    // send message if terminated abruptly
                    if (!WIFEXITED(childStatus))
                    {
                        printf("Child %d exited abnormally due to signal %d\n", childPid, WTERMSIG(childStatus));
                    }
                }
                break;
            }
        }
        else
        {
            // if input surpass character count
            printf("too many characters");
            continue;
        }
    }
}