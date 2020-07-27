#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <signal.h>

// Takes in user Input and returns it as a char *array
char *readInput()
{
    char *buffer = malloc(sizeof(char) * 1024);
    char c;
    int index = 0;

    if (!buffer)
    {
        fprintf(stderr, "Ran out of memory or some memory error\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();
        if (c == '\r')
        {
            continue;
        }
        if (c == EOF || c == '\n')
        {
            buffer[index] = '\0';
            return buffer;
        }
        buffer[index] = c;
        index++;
        if (index == 1024)
        {
            fprintf(stderr, "User attempted to overflow buffer. Aborting Program.");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
}

// Parses the input into seperate arguments and returns a char **
char **parseInput(char *userCommand)
{
    char **args;
    args = calloc(2, sizeof(char *));
    char *delimeters = " \n\t\r";
    char *argument;
    argument = strtok(userCommand, delimeters);
    int index = 0;

    while (argument != NULL)
    {
        args[index] = argument;
        argument = strtok(NULL, delimeters);
        index++;
        if (index > 2)
        {
            args = NULL;
            return args;
        }
    }
    if (index < 2)
    {
        args = NULL;
    }
    return args;
}

// Reads a file and returns it as char *
char *readFile(const char *path)
{
    /* DECLARE VARIABLES*/
    FILE *fp;
    char *buffer;
    off_t numbytes;
    /* ATTEMPT TO OPEN FILE QUIT IF NULL*/
    if ((fp = fopen(path, "r")) == NULL)
    {
        return NULL;
    }

    /* MOVE THE FILE POINTER UNTIL IT REACHES THE END OF THE FILE
       SET THE NUMBER OF BYTES UNTIL END OF FILE BY CALLNIG FTELL */
    fseeko(fp, 0, SEEK_END);
    numbytes = ftello(fp);

    /* SET FILE INDICATOR TO THE START OF THE FILE*/
    fseeko(fp, 0L, SEEK_SET);

    /* ALLOCATE ENOUGH MEMORY TO HOLD ALL CONTENTS OF THE FILE DETERMINDED BY NUMBYTES*/
    buffer = (char *)calloc(numbytes, sizeof(char));

    /* COPY DATA*/
    fread(buffer, sizeof(char), numbytes, fp);

    /* CLOSE FILE */
    (void)fclose(fp);

    return buffer;
}

int main(void)
{

    // Declare file descriptors for process communication
    int pipefds1[2], pipefds2[2];
    pipe(pipefds1);
    pipe(pipefds2);

    int *status = NULL;

    // Declare child processes
    pid_t child_a, child_b;

    // Create first child process
    child_a = fork();

    // /* Child A code */
    if (child_a == 0)
    {
        // /* Print process & CPU Information */

        // Close unused file descriptors
        close(pipefds1[1]);
        close(pipefds2[0]);

        // Create an array to hold process id
        char s_process[1024 - strlen("PROCESS ")];

        // Read the process id from the parent
        read(pipefds1[0], s_process, 1024 - strlen("PROCESS "));

        // Declare a file descriptor
        FILE *jsonptr;

        // Open a file in write mode
        jsonptr = fopen("results.json", "w");

        // Write to file
        fprintf(jsonptr, "{\"results\": [{\"sent\": \"%s\",\"receive\": \"", s_process);

        // Convert process to an integer
        int process = atoi(s_process);

        // Determine if the process is real
        if (kill(process, 0) == 0)
        {
            fprintf(jsonptr, "PID[%d]: Process ID: %d is an active process.\"", getpid(), process);
        }

        else
        {
            fprintf(jsonptr, "PID[%d]: Process ID: %d is not an active process.", getpid(), process);
        }
        fprintf(jsonptr, "}]}");
        fclose(jsonptr);
    }
    else
    {
        // Second child process created
        child_b = fork();

        if (child_b == 0)
        {
            /* Child B code */
            /* File Path */

            // Close unused file descriptors
            close(pipefds1[1]);
            close(pipefds2[0]);

            // Create an array to store the file path
            char filePath[1024 - strlen("PATH ")];

            // Read path from the parent process
            read(pipefds1[0], filePath, 1024 - strlen("PATH "));

            // Create a buffer to store file contents if it exists
            char *buffer = readFile(filePath);

            // Determine if the path is vaild

            if (buffer != NULL)
            {
                printf("PID[%d]: File Path %s is a valid file path.\n\n", getpid(), filePath);
            }
            else
            {
                printf("PID[%d]: File Path %s is not a valid file path.\n\n", getpid(), filePath);
            }
            fflush(stdout);
            fflush(stdout);
            FILE *jsonptr;
            jsonptr = fopen("results.json", "w");
            fprintf(jsonptr, "{\"results\": [{\"sent\": \"%s\",\"receive\": \"%s\"}]}", filePath, buffer);
            fclose(jsonptr);
            free(buffer);
        }
        /*Parent Process */
        else
        {

            // CLOSE FDs WE WONT USE
            close(pipefds1[0]);
            close(pipefds2[1]);

            // Store user input
            char *userCommand;

            // Store user input parsed
            char **args;

            printf("Enter \"PATH PATH_NAME\" to determine if provided PATH_NAME is a valid path.\n");
            printf("Example: \"PATH C:\\Users\\Bob\\Documents\\text_file.txt\"\n\n");
            printf("Enter \"PROCESS PROCESS_ID\" to find CPU information about the provided PROCESS_ID.\n");
            printf("Example: \"PROCESS 4521\"\n\n");
            fflush(stdout);

            userCommand = readInput();

            args = parseInput(userCommand);

            if (args != NULL)
            {
                // If we are passing path to a child process
                if (strcmp(args[0], "PATH") == 0 && strlen(args[0]) == 4)
                {
                    kill(child_a, SIGKILL); // kill the alternative child process
                    printf("\nPID[%d]: Attempting to locate file: %s....\n\n", getpid(), args[1]);
                    printf("\nPID[%d]: Passing file path to child process %d.\n\n", getpid(), child_b);
                    fflush(stdout);
                    write(pipefds1[1], args[1], (1024 - strlen("PATH "))); // send the path to the child process
                    waitpid(child_b, status, 0);                           // wait for the child to respond
                    sleep(3);
                }
                // If we are passing process id to a child process
                else if (strcmp(args[0], "PROCESS") == 0 && strlen(args[0]) == 7)
                {
                    kill(child_b, SIGKILL); // kill the alternative child process
                    printf("\nPID[%d]: Determining if %s is a running process....\n\n", getpid(), args[1]);
                    printf("\nPID[%d]: Passing process number to child process %d.\n\n", getpid(), child_a);
                    fflush(stdout);
                    write(pipefds1[1], args[1], (1024 - strlen("PROCESS "))); // send the process to the child process
                    waitpid(child_a, status, 0);                              // wait for the child to respond
                    sleep(3);
                }
                printf("Results located in the file \"results.json\".\n\nProgram terminated successfully\n");
            }
            else // User did not adhere to entering the proper input
            {
                printf("\nUser did not enter input properly\n\n");
                printf("No results provided in the file \"results.json\".\n\nProgram terminated early\n");
                fflush(stdout);
            }
            kill(child_a, SIGKILL);
            kill(child_b, SIGKILL);
            free(userCommand);
            free(args);
        }
    }
    return EXIT_SUCCESS;
}