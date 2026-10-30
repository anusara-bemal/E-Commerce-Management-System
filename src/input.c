#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool readLine(const char *prompt, char *buffer, size_t size)
{
    if (buffer == NULL || size < 2)
        return false;

    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
        return false;

    size_t length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
        return true;
    }

    /* Detect input longer than the buffer. */
    int ch = getchar();

    if (ch != '\n' && ch != EOF) {
        while (ch != '\n' && ch != EOF)
            ch = getchar();

        printf("Input is too long. Please try again.\n");
        return false;
    }

    return true;
}

bool readNonEmptyString(const char *prompt, char *buffer, size_t size)
{
    while (true) {
        if (!readLine(prompt, buffer, size))
            return false;

        char *start = buffer;

        while (isspace((unsigned char)*start))
            start++;

        if (*start == '\0') {
            printf("Input cannot be empty. Please try again.\n");
            continue;
        }

        if (start != buffer)
            memmove(buffer, start, strlen(start) + 1);

        return true;
    }
}

bool readInteger(const char *prompt, int *value, int min, int max)
{
    char buffer[128];

    if (value == NULL || min > max)
        return false;

    while (true) {
        if (!readLine(prompt, buffer, sizeof(buffer)))
            return false;

        char *start = buffer;
        while (isspace((unsigned char)*start))
            start++;

        if (*start == '\0') {
            printf("Please enter a number.\n");
            continue;
        }

        errno = 0;
        char *end;
        long number = strtol(start, &end, 10);

        while (isspace((unsigned char)*end))
            end++;

        if (errno == ERANGE || end == start || *end != '\0' ||
            number < min || number > max) {
            printf("Invalid number. Enter a value between %d and %d.\n",
                   min, max);
            continue;
        }

        *value = (int)number;
        return true;
    }
}

bool readDouble(const char *prompt, double *value, double min, double max)
{
    char buffer[128];

    if (value == NULL || min > max)
        return false;

    while (true) {
        if (!readLine(prompt, buffer, sizeof(buffer)))
            return false;

        char *start = buffer;
        while (isspace((unsigned char)*start))
            start++;

        if (*start == '\0') {
            printf("Please enter a number.\n");
            continue;
        }

        errno = 0;
        char *end;
        double number = strtod(start, &end);

        while (isspace((unsigned char)*end))
            end++;

        if (errno == ERANGE || end == start || *end != '\0' ||
            number != number || number < min || number > max) {
            printf("Invalid amount. Enter a value between %.2f and %.2f.\n",
                   min, max);
            continue;
        }

        *value = number;
        return true;
    }
}

