#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "input.h"

#include "models.h"
#include "users.h"

/* Generate the next user ID */
static int getNextUserId(void)
{
    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return -1;
    }

    char line[500];
    int lastId = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        int id;

        if (sscanf(line, "%d|", &id) == 1 && id > 0)
        {
            if (id > lastId)
                lastId = id;
        }
    }

    if (ferror(fp))
    {
        printf("Error reading users file.\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    if (lastId == INT_MAX)
    {
        printf("Cannot generate another user ID.\n");
        return -1;
    }

    return lastId + 1;
}


/* Login */
int login(char logginuser[])
{
    char intuser[MAX_USERNAME];
    char intpass[MAX_PASSWORD];

    printf("==================================================\n");
    printf("LOGIN\n");
    printf("==================================================\n");

    printf("Enter Username: ");

    if (fgets(intuser, sizeof(intuser), stdin) == NULL)
        return 0;

    intuser[strcspn(intuser, "\r\n")] = '\0';

    printf("Enter Password: ");

    if (fgets(intpass, sizeof(intpass), stdin) == NULL)
        return 0;

    intpass[strcspn(intpass, "\r\n")] = '\0';

    if (intuser[0] == '\0' || intpass[0] == '\0')
    {
        printf("Username and password cannot be empty.\n");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return 0;
    }

    char line[500];
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *id = strtok(line, "|");
        char *username = strtok(NULL, "|");
        char *password = strtok(NULL, "|");
        char *email = strtok(NULL, "|");
        char *phone = strtok(NULL, "|");
        char *address = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *active = strtok(NULL, "|");

        if (id == NULL || username == NULL || password == NULL ||
            email == NULL || phone == NULL || address == NULL ||
            role == NULL || active == NULL)
        {
            continue;
        }

        if (strcmp(username, intuser) == 0 &&
            strcmp(password, intpass) == 0)
        {
            // if (strcmp(active, "1") != 0)
            // {
            //     printf("This account is inactive.\n");
            //     break;
            // }

            if (strlen(username) >= MAX_USERNAME)
                break;

            strcpy(logginuser, username);
            found = 1;
            break;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        printf("Error reading users file.\n");
        return 0;
    }

    fclose(fp);

    if (found)
    {
        printf("\nLogin successful!\n");
        return 1;
    }

    printf("\nInvalid username or password.\n");
    return 0;
}


/* Validate username */
int validuser(char username[])
{
    if (strlen(username) < 3)
    {
        printf("Username must have at least 3 characters.\n");
        return 0;
    }

    if (strchr(username, '|') != NULL)
    {
        printf("Username cannot contain '|'.\n");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return 0;
    }

    char line[500];

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *id = strtok(line, "|");
        char *existingUsername = strtok(NULL, "|");

        if (id != NULL && existingUsername != NULL &&
            strcmp(existingUsername, username) == 0)
        {
            fclose(fp);
            printf("Username already exists!\n");
            return 0;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        printf("Error reading users file.\n");
        return 0;
    }

    fclose(fp);
    return 1;
}

/* Register customer */
void registerCustomer(void)
{
    User d = {0};

    printf("==================================================\n");
    printf("CUSTOMER REGISTRATION\n");
    printf("==================================================\n");

    /* Username validation */
    while (1)
    {
        if (!readNonEmptyString(
                "Username: ", d.username, sizeof(d.username)))
        {
            return;
        }

        if (validuser(d.username))
        {
            break;
        }

        printf("Please enter a different valid username.\n");
    }

    /* Password validation */
    while (1)
    {
        if (!readNonEmptyString(
                "Password: ", d.password, sizeof(d.password)))
        {
            return;
        }

        if (strchr(d.password, '|') == NULL)
        {
            break;
        }

        printf("Password cannot contain '|'. Please try again.\n");
    }

    /* Email validation */
    while (1)
    {
        if (!readNonEmptyString(
                "Email: ", d.email, sizeof(d.email)))
        {
            return;
        }

        char *at = strchr(d.email, '@');

        if (at != NULL &&
            at != d.email &&
            strchr(at + 1, '.') != NULL &&
            strchr(d.email, '|') == NULL)
        {
            break;
        }

        printf("Invalid email address. Please try again.\n");
    }

    /* Phone number validation */
    while (1)
    {
        if (!readNonEmptyString(
                "Phone Number (10 digits): ",
                d.tpNumber, sizeof(d.tpNumber)))
        {
            return;
        }

        int validPhone = (strlen(d.tpNumber) == 10);

        for (size_t i = 0; d.tpNumber[i] != '\0'; i++)
        {
            if (d.tpNumber[i] < '0' || d.tpNumber[i] > '9')
            {
                validPhone = 0;
                break;
            }
        }

        if (validPhone)
        {
            break;
        }

        printf("Phone number must contain exactly 10 digits.\n");
    }

    /* Address validation */
    while (1)
    {
        if (!readNonEmptyString(
                "Address: ", d.address, sizeof(d.address)))
        {
            return;
        }

        if (strchr(d.address, '|') == NULL)
        {
            break;
        }

        printf("Address cannot contain '|'. Please try again.\n");
    }

    /* Generate user ID */
    d.id = getNextUserId();

    if (d.id == -1)
    {
        printf("Registration failed. Cannot generate user ID.\n");
        return;
    }

    d.role = ROLE_CUSTOMER;
    d.isActive = 1;

    /* Save customer */
    FILE *fp = fopen(USERS_FILE, "a");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return;
    }

    int result = fprintf(
        fp, "%d|%s|%s|%s|%s|%s|%d|%d\n",
        d.id,
        d.username,
        d.password,
        d.email,
        d.tpNumber,
        d.address,
        d.role,
        d.isActive
    );

    if (result < 0)
    {
        fclose(fp);
        printf("Error saving registration data.\n");
        return;
    }

    if (fclose(fp) != 0)
    {
        printf("Error closing users file.\n");
        return;
    }

    printf("\nCustomer registered successfully! ID = %d\n", d.id);
}

/* Update profile */
void updateprofile(char logginuser[])
{
    logginuser[strcspn(logginuser, "\r\n")] = '\0';

    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return;
    }

    FILE *temp = fopen("temp.txt", "w");

    if (temp == NULL)
    {
        fclose(fp);
        printf("Cannot create temporary file.\n");
        return;
    }

    char line[500];
    int found = 0;
    int error = 0;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char copy[500];
        strcpy(copy, line);

        char *id       = strtok(copy, "|");
        char *username = strtok(NULL, "|");
        char *password = strtok(NULL, "|");
        char *email    = strtok(NULL, "|");
        char *phone    = strtok(NULL, "|");
        char *address  = strtok(NULL, "|");
        char *role     = strtok(NULL, "|");
        char *active   = strtok(NULL, "|");

        if (id == NULL || username == NULL || password == NULL ||
            email == NULL || phone == NULL || address == NULL ||
            role == NULL || active == NULL)
        {
            if (fprintf(temp, "%s\n", line) < 0)
            {
                error = 1;
                break;
            }

            continue;
        }

        if (strcmp(username, logginuser) != 0)
        {
            if (fprintf(temp, "%s\n", line) < 0)
            {
                error = 1;
                break;
            }

            continue;
        }

        User d = {0};

        snprintf(d.username, sizeof(d.username), "%s", username);
        snprintf(d.password, sizeof(d.password), "%s", password);
        snprintf(d.email, sizeof(d.email), "%s", email);
        snprintf(d.tpNumber, sizeof(d.tpNumber), "%s", phone);
        snprintf(d.address, sizeof(d.address), "%s", address);

        printf("\n========== UPDATE PROFILE ==========\n");
        printf("Current Email: %s\n", d.email);
        printf("Current Phone: %s\n", d.tpNumber);
        printf("Current Address: %s\n", d.address);

        /* Email validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "New Email: ", d.email, sizeof(d.email)))
            {
                error = 1;
                break;
            }

            char *at = strchr(d.email, '@');

            if (at != NULL &&
                at != d.email &&
                strchr(at + 1, '.') != NULL &&
                strchr(d.email, '|') == NULL)
            {
                break;
            }

            printf("Invalid email address. Please try again.\n");
        }

        if (error)
            break;

        /* Phone validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "New Phone Number (10 digits): ",
                    d.tpNumber, sizeof(d.tpNumber)))
            {
                error = 1;
                break;
            }

            int validPhone = (strlen(d.tpNumber) == 10);

            for (size_t i = 0; d.tpNumber[i] != '\0'; i++)
            {
                if (d.tpNumber[i] < '0' || d.tpNumber[i] > '9')
                {
                    validPhone = 0;
                    break;
                }
            }

            if (validPhone)
                break;

            printf("Phone number must contain exactly 10 digits.\n");
        }

        if (error)
            break;

        /* Address validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "New Address: ", d.address, sizeof(d.address)))
            {
                error = 1;
                break;
            }

            if (strchr(d.address, '|') == NULL)
                break;

            printf("Address cannot contain '|'. Please try again.\n");
        }

        if (error)
            break;

        if (fprintf(temp, "%s|%s|%s|%s|%s|%s|%s|%s\n",
                    id,
                    d.username,
                    d.password,
                    d.email,
                    d.tpNumber,
                    d.address,
                    role,
                    active) < 0)
        {
            error = 1;
            break;
        }

        found = 1;
    }

    if (ferror(fp))
        error = 1;

    if (fclose(fp) != 0)
        error = 1;

    if (fclose(temp) != 0)
        error = 1;

    if (error)
    {
        remove("temp.txt");
        printf("\nProfile update failed. Original data was preserved.\n");
        return;
    }

    if (!found)
    {
        remove("temp.txt");
        printf("User not found: %s\n", logginuser);
        return;
    }

    if (rename("temp.txt", USERS_FILE) != 0)
    {
        printf("Could not replace the users file.\n");
        printf("Temporary file retained: temp.txt\n");
        return;
    }

    printf("\nProfile updated successfully!\n");
}


/* Logout */
void logout(char logginuser[])
{
    logginuser[0] = '\0';
    printf("Logout successful!\n");
}


int getUserRole(char username[])
{
    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
        return -1;

    char line[500];
    int role = -1;

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *id       = strtok(line, "|");
        char *name     = strtok(NULL, "|");
        char *password = strtok(NULL, "|");
        char *email    = strtok(NULL, "|");
        char *phone    = strtok(NULL, "|");
        char *address  = strtok(NULL, "|");
        char *userRole = strtok(NULL, "|");
        char *active   = strtok(NULL, "|");

        if (id == NULL || name == NULL || password == NULL ||
            email == NULL || phone == NULL || address == NULL ||
            userRole == NULL || active == NULL)
        {
            continue;
        }

        if (strcmp(name, username) == 0 && strcmp(active, "1") == 0)
        {
            role = atoi(userRole);
            break;
        }
    }

    fclose(fp);
    return role;
}

