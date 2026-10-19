#ifndef USERS_H
#define USERS_H

#include "models.h"

int login(char logginuser[]);
void registerCustomer(void);
int validuser(char username[]);
void updateprofile(char logginuser[]);
void logout(char logginuser[]);
int getUserRole(char username[]);

#endif