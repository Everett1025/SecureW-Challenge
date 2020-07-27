
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <assert.h>
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

    // Tests to make sure input is being stored correctly
    // When entering strings, enter the string literals you see in the assert
    char *testInput = readInput();
    assert(strcmp(testInput, "Here is some random input") == 0);

    testInput = readInput();
    assert(strcmp(testInput, "12345") == 0);

    // Tests when reading a file with text
    char *testfile = readFile("FileWithText.txt");
    assert((strcmp(testfile, "Hey, this file had some text in it.\nTesting is fun.") == 0));

    // Tests when reading a file without text
    char *testfile2 = readFile("EmptyText.txt");
    assert((strcmp(testfile2, "") == 0));
}