#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stddef.h>

bool readLine(const char *prompt, char *buffer, size_t size);
bool readNonEmptyString(const char *prompt, char *buffer, size_t size);
bool readInteger(const char *prompt, int *value, int min, int max);
bool readDouble(const char *prompt, double *value, double min, double max);

#endif

