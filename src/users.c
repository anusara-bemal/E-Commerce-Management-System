#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


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

    /* Username */
    printf("Username: ");

    if (fgets(d.username, sizeof(d.username), stdin) == NULL)
        return;

    if (strchr(d.username, '\n') == NULL &&
        strchr(d.username, '\r') == NULL &&
        !feof(stdin))
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }

        printf("Username is too long.\n");
        return;
    }

    d.username[strcspn(d.username, "\r\n")] = '\0';

    if (!validuser(d.username))
        return;

    /* Password */
    printf("Password: ");

    if (fgets(d.password, sizeof(d.password), stdin) == NULL)
        return;

    if (strchr(d.password, '\n') == NULL &&
        strchr(d.password, '\r') == NULL &&
        !feof(stdin))
    {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }

        printf("Password is too long.\n");
        return;
    }

    d.password[strcspn(d.password, "\r\n")] = '\0';

    if (strlen(d.password) == 0 ||
        strchr(d.password, '|') != NULL)
    {
        printf("Password cannot be empty or contain '|'.\n");
        return;
    }

    /* Email */
    printf("Email: ");

    if (fgets(d.email, sizeof(d.email), stdin) == NULL)
        return;

    d.email[strcspn(d.email, "\r\n")] = '\0';

    if (strchr(d.email, '@') == NULL ||
        strchr(d.email, '|') != NULL)
    {
        printf("Invalid email address.\n");
        return;
    }

    /* Phone */
    printf("Phone Number: ");

    if (fgets(d.tpNumber, sizeof(d.tpNumber), stdin) == NULL)
        return;

    d.tpNumber[strcspn(d.tpNumber, "\r\n")] = '\0';

    if (strlen(d.tpNumber) != 10)
    {
        printf("Phone number must contain exactly 10 digits.\n");
        return;
    }

    for (int i = 0; d.tpNumber[i] != '\0'; i++)
    {
        if (d.tpNumber[i] < '0' || d.tpNumber[i] > '9')
        {
            printf("Phone number must contain digits only.\n");
            return;
        }
    }

    /* Address */
    printf("Address: ");

    if (fgets(d.address, sizeof(d.address), stdin) == NULL)
        return;

    d.address[strcspn(d.address, "\r\n")] = '\0';

    if (strlen(d.address) == 0 ||
        strchr(d.address, '|') != NULL)
    {
        printf("Address cannot be empty or contain '|'.\n");
        return;
    }

    /* Generate ID */
    d.id = getNextUserId();

    if (d.id == -1)
    {
        printf("Registration failed. Cannot generate user ID.\n");
        return;
    }

    d.role = ROLE_CUSTOMER;
    d.isActive = 1;

    /* Save user */
    FILE *fp = fopen(USERS_FILE, "a");

    if (fp == NULL)
    {
        printf("Cannot open file: %s\n", USERS_FILE);
        return;
    }

    int result = fprintf(fp, "%d|%s|%s|%s|%s|%s|%d|%d\n",
                         d.id,
                         d.username,
                         d.password,
                         d.email,
                         d.tpNumber,
                         d.address,
                         d.role,
                         d.isActive);

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

        char *id = strtok(copy, "|");
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
            if (fprintf(temp, "%s\n", line) < 0)
            {
                error = 1;
                break;
            }

            continue;
        }

        if (strcmp(username, logginuser) == 0)
        {
            User d = {0};

            strcpy(d.username, username);
            strcpy(d.password, password);
            strcpy(d.email, email);
            strcpy(d.tpNumber, phone);
            strcpy(d.address, address);

            printf("\n========== UPDATE PROFILE ==========\n");
            printf("Current Email: %s\n", d.email);

            /* New email */
            printf("New Email: ");

            if (fgets(d.email, sizeof(d.email), stdin) == NULL)
            {
                error = 1;
                break;
            }

            d.email[strcspn(d.email, "\r\n")] = '\0';

            if (strchr(d.email, '@') == NULL ||
                strchr(d.email, '|') != NULL)
            {
                printf("Invalid email address.\n");
                error = 1;
                break;
            }

            /* New phone */
            printf("New Phone Number: ");

            if (fgets(d.tpNumber, sizeof(d.tpNumber), stdin) == NULL)
            {
                error = 1;
                break;
            }

            d.tpNumber[strcspn(d.tpNumber, "\r\n")] = '\0';

            if (strlen(d.tpNumber) != 10)
            {
                printf("Phone number must contain exactly 10 digits.\n");
                error = 1;
                break;
            }

            int validPhone = 1;

            for (int i = 0; d.tpNumber[i] != '\0'; i++)
            {
                if (d.tpNumber[i] < '0' || d.tpNumber[i] > '9')
                {
                    validPhone = 0;
                    break;
                }
            }

            if (!validPhone)
            {
                printf("Phone number must contain digits only.\n");
                error = 1;
                break;
            }

            /* New address */
            printf("New Address: ");

            if (fgets(d.address, sizeof(d.address), stdin) == NULL)
            {
                error = 1;
                break;
            }

            d.address[strcspn(d.address, "\r\n")] = '\0';

            if (strlen(d.address) == 0 ||
                strchr(d.address, '|') != NULL)
            {
                printf("Address cannot be empty or contain '|'.\n");
                error = 1;
                break;
            }

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
        else
        {
            if (fprintf(temp, "%s\n", line) < 0)
            {
                error = 1;
                break;
            }
        }
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
        printf("Profile update failed. Original data was preserved.\n");
        return;
    }

    if (!found)
    {
        remove("temp.txt");
        printf("User not found: %s\n", logginuser);
        return;
    }

    /*
       Replace the original file.
       On some systems, rename() cannot replace an existing file.
       In that case, the original file remains untouched.
    */
    if (rename("temp.txt", USERS_FILE) != 0)
    {
        printf("Could not replace the users file.\n");
        printf("Original data was preserved. Temporary file: temp.txt\n");
        return;
    }

    printf("Profile updated successfully!\n");
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

