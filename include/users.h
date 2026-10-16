#ifndef USERS_H
#define USERS_H

#include "models.h"

// Public functions
int login(char logginuser[]);
void registerCustomer(void);
int validuser(char username[]);
void updateprofile(char logginuser[]);
void logout(char logginuser[]);

#endif