#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <signal.h>

// Reads a file and returns it as char *
char *readFile(const char *path, off_t *numbytes)
{
    /* DECLARE VARIABLES*/
    FILE *fp;
    //off_t numbytes;
    char *buffer;
    /* ATTEMPT TO OPEN FILE QUIT IF NULL*/
    if ((fp = fopen(path, "r")) == NULL)
    {
        return NULL;
    }

    /* MOVE THE FILE POINTER UNTIL IT REACHES THE END OF THE FILE
       SET THE NUMBER OF BYTES UNTIL END OF FILE BY CALLNIG FTELL */
    fseeko(fp, 0, SEEK_END);
    *numbytes = ftello(fp);

    /* SET FILE INDICATOR TO THE START OF THE FILE*/
    fseeko(fp, 0L, SEEK_SET);

    /* ALLOCATE ENOUGH MEMORY TO HOLD ALL CONTENTS OF THE FILE DETERMINDED BY NUMBYTES*/
    buffer = (char *)calloc(*numbytes + 1, sizeof(char));

    /* COPY DATA*/
    fread(buffer, sizeof(char), *numbytes, fp);

    /* CLOSE FILE */
    (void)fclose(fp);

    return buffer;
}

// Takes in user Input and returns it as a char *array
char *readInput(long *inputSize)
{
    size_t cap = 1024; /* Initial capacity for the char buffer */
    size_t len = 0;    /* Current offset of the buffer */

    char *buffer = malloc(cap * sizeof(char));

    int c;

    /* Read char by char, breaking if we reach EOF or a newline */
    while ((c = fgetc(stdin)) != '\n' && !feof(stdin))
    {
        buffer[len] = c;

        /* When cap == len, we need to resize the buffer
       * so that we don't overwrite any bytes
       */
        if (++len == cap)
        {
            /* Make the output buffer twice its current size */
            buffer = realloc(buffer, (cap *= 2) * sizeof(char));
        }
    }

    /* Trim off any unused bytes from the buffer */
    buffer = realloc(buffer, (len + 1) * sizeof(char));

    /* Pad the last byte so we don't overread the buffer in the future */
    buffer[len] = '\0';

    *inputSize = len + 1;

    return buffer;
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

void printInfoMessage()
{
    printf("Enter \"PATH PATH_NAME\" to determine if provided PATH_NAME is a valid path.\n");
    printf("Example: \"PATH C:\\Users\\Bob\\Documents\\text_file.txt\"\n\n");
    printf("Enter \"PROCESS PROCESS_ID\" to find CPU information about the provided PROCESS_ID.\n");
    printf("Example: \"PROCESS 4521\"\n\n");
    fflush(stdout);
}

void wrongInputMessage()
{
    printf("\nUser did not enter input properly\n\n");
    printf("No results provided in the file \"results.json\".\n\nProgram terminated early\n");
    fflush(stdout);
}

void findingProcessMessage(char **args, pid_t child_a)
{
    printf("\nPID[%d]: Determining if %s is a running process....\n\n", getpid(), args[1]);
    printf("\nPID[%d]: Passing process number to child process %d.\n\n", getpid(), child_a);
    fflush(stdout);
}

void findingPathMessage(char **args, pid_t child_b)
{
    printf("\nPID[%d]: Attempting to locate file: %s....\n\n", getpid(), args[1]);
    printf("\nPID[%d]: Passing file path to child process %d.\n\n", getpid(), child_b);
    fflush(stdout);
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

        long messageLen = 0;

        read(pipefds1[0], &messageLen, sizeof(messageLen));

        // Create an array to hold process id
        char s_process[messageLen];

        // Read the process id from the parent
        read(pipefds1[0], s_process, messageLen);

        // Convert process to an integer
        int process = atoi(s_process);
        int realProcess = 0;

        // Determine if the process is real
        if (kill(process, 0) == 0)
        {
            realProcess = 1;
        }

        // SET UP FOR EXECL CALL TOP
        char *binaryPath = "/bin/mpstat";
        // char *arg1 = "-p";
        // char *arg2 = s_process;

        /*
        int saved_stdout = dup(1);

        int pipes[2];

        pipe(pipes); // Create the pipes

        dup2(pipes[1], 1); // Set the pipe up to standard output

        execl(binaryPath, binaryPath, NULL);

        //freopen("output.txt", "w", stdout);

        // FILE *mpstat = fdopen(pipes[0], "r");

        char mpstatData[2000];

        read(pipes[0], mpstatData, 2000);
*/
        // /* COPY DATA*/
        // fread(mpstatData, sizeof(char), 2000, mpstat);

        // /* CLOSE FILE */
        //(void)fclose(mpstat);
        /*
        dup2(saved_stdout, 1);
        close(s)
        // close(pipes[0]);
        // close(pipes[1]);

        printf("mpstat bruh%s\n", mpstatData);
        */
        // Open a file in write mode
        jsonptr = fopen("results.json", "w");

        // Write to file
        fprintf(jsonptr, "{\"results\": [{\"sent\": \"%s\",\"receive\": \"", s_process);

        if (realProcess)
        {
            fprintf(jsonptr, "PID[%d]: Process ID: %d is an active process.\"", getpid(), process);
        }
        else
        {
            fprintf(jsonptr, "PID[%d]: Process ID: %d is not an active process.", getpid(), process);
        }

        fprintf(jsonptr, "}]}");
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

            long messageLen = 0;

            read(pipefds1[0], &messageLen, sizeof(messageLen));

            // Create an array to the path
            char filePath[messageLen];

            // Read the process id from the parent
            read(pipefds1[0], filePath, messageLen);

            // Create a buffer to store file contents if it exists
            off_t fileBytes;
            char *fileContents = readFile(filePath, &fileBytes);

            // Determine if the path is vaild

            if (fileContents != NULL)
            {
                printf("PID[%d]: File Path %s is a valid file path.\n\n", getpid(), filePath);
            }
            else
            {
                printf("PID[%d]: File Path %s is not a valid file path.\n\n", getpid(), filePath);
            }
            fflush(stdout);
            FILE *jsonptr;
            jsonptr = fopen("results.json", "w");
            fprintf(jsonptr, "{\"results\": [{\"sent\": \"%s\",\"receive\": \"%s\"}]}", filePath, fileContents);
            fclose(jsonptr);
            free(fileContents);
        }
        /*Parent Process */
        else
        {

            // CLOSE FDs WE WONT USE
            close(pipefds1[0]);
            close(pipefds2[1]);

            // Store user input
            char *userInput;
            long inputLength;

            printInfoMessage();

            userInput = readInput(&inputLength);

            // Store user input parsed
            char **args;
            long *secondArgLength;
            args = parseInput(userInput);

            long argsOneLength = strlen(args[1]) + 1;

            if (args != NULL)
            {
                // If we are passing path to a child process
                if (strcmp(args[0], "PATH") == 0 && strlen(args[0]) == 4)
                {
                    kill(child_a, SIGKILL); // kill the alternative child process
                    findingPathMessage(args, child_b);
                    write(pipefds1[1], &argsOneLength, sizeof(argsOneLength));
                    write(pipefds1[1], args[1], argsOneLength);
                    waitpid(child_b, status, 0); // wait for the child to respond
                    sleep(3);
                }
                // If we are passing process id to a child process
                else if (strcmp(args[0], "PROCESS") == 0 && strlen(args[0]) == 7)
                {
                    kill(child_b, SIGKILL); // kill the alternative child process
                    findingProcessMessage(args, child_a);
                    write(pipefds1[1], &argsOneLength, sizeof(argsOneLength));
                    write(pipefds1[1], args[1], argsOneLength);
                    waitpid(child_a, status, 0); // wait for the child to respond
                    sleep(3);
                }
                printf("Results located in the file \"results.json\".\n\nProgram terminated successfully\n");
            }
            else // User did not adhere to entering the proper input
            {
                wrongInputMessage();
            }
            kill(child_a, SIGKILL);
            kill(child_b, SIGKILL);
            free(userInput);
            free(args);
        }
    }
    return EXIT_SUCCESS;
}

// /mnt/c/Users/Everett/Documents/Personal/Hacking/blah.txt