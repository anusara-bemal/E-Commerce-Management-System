#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "input.h"
#include "ui.h"
#include "models.h"
#include "users.h"

/* Generate the next user ID */
static int getNextUserId(void)
{
    FILE *fp = fopen(USERS_FILE, "r");

    if (fp == NULL)
    {
        
        printf(RED "Cannot open file: %s\n" RESET, USERS_FILE);
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
        printf(RED "Error reading users file.\n" RESET);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    if (lastId == INT_MAX)
    {
        printf(RED "Cannot generate another user ID.\n" RESET);
        return -1;
    }

    return lastId + 1;
}


/* Login */
int login(char logginuser[])
{
    char intuser[MAX_USERNAME];
    char intpass[MAX_PASSWORD];

    clearScreen();
    drawHeader("CUSTOMER LOGIN");
    printf("\n  " DIM "Enter your credentials to continue" RESET "\n\n");

    printf("  " CYAN "❯" RESET " Enter Username: ");
    if (fgets(intuser, sizeof(intuser), stdin) == NULL)
        return 0;
    intuser[strcspn(intuser, "\r\n")] = '\0';

    printf("  " CYAN "❯" RESET " Enter Password: ");
    if (fgets(intpass, sizeof(intpass), stdin) == NULL)
        return 0;
    intpass[strcspn(intpass, "\r\n")] = '\0';

    if (intuser[0] == '\0' || intpass[0] == '\0')
    {
        showError("Username and password cannot be empty.");
        pressEnterToContinue();
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "Cannot open file: %s", USERS_FILE);
        showError(msg);
        pressEnterToContinue();
        return 0;
    }

    char line[500];
    int found = 0;

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
            continue;
        }

        if (strcmp(username, intuser) == 0 &&
            strcmp(password, intpass) == 0)
        {
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
        showError("Error reading users file.");
        pressEnterToContinue();
        return 0;
    }

    fclose(fp);

    if (found)
    {
        printf("\n");
        showSuccess("Login successful! Welcome back.");
        return 1;
    }

    showError("Invalid username or password.");
    pressEnterToContinue();
    return 0;
}

/* Validate username */
int validuser(char username[])
{
    if (strlen(username) < 3)
    {
        printf(YELLOW "Username must have at least 3 characters.\n" RESET);
        return 0;
    }

    if (strchr(username, '|') != NULL)
    {
        printf(YELLOW "Username cannot contain '|'.\n" RESET);
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
            printf(RED "Username already exists!\n" RESET);
            return 0;
        }
    }

    if (ferror(fp))
    {
        fclose(fp);
        printf(RED "Error reading users file.\n" RESET);
        return 0;
    }

    fclose(fp);
    return 1;
}

/* Register customer - Polished Nordic UI */
void registerCustomer(void)
{
    User d = {0};

    clearScreen();
    drawHeader("CUSTOMER REGISTRATION");
    printf("\n  " DIM "Create your account - All fields required" RESET "\n\n");

    /* Username validation */
    while (1)
    {
        if (!readNonEmptyString(
                "  " CYAN "❯" RESET " Username: ", d.username, sizeof(d.username)))
        {
            return;
        }

        if (validuser(d.username))
        {
            break;
        }

        showWarning("Please enter a different valid username.");
    }

    /* Password validation */
    while (1)
    {
        if (!readNonEmptyString(
                "  " CYAN "❯" RESET " Password: ", d.password, sizeof(d.password)))
        {
            return;
        }

        if (strchr(d.password, '|') == NULL)
        {
            break;
        }

        showWarning("Password cannot contain '|'. Please try again.");
    }

    /* Email validation */
    while (1)
    {
        if (!readNonEmptyString(
                "  " CYAN "❯" RESET " Email: ", d.email, sizeof(d.email)))
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

        showWarning("Invalid email address. Please try again.");
    }

    /* Phone number validation */
    while (1)
    {
        if (!readNonEmptyString(
                "  " CYAN "❯" RESET " Phone Number (10 digits): ",
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

        showWarning("Phone number must contain exactly 10 digits.");
    }

    /* Address validation */
    while (1)
    {
        if (!readNonEmptyString(
                "  " CYAN "❯" RESET " Address: ", d.address, sizeof(d.address)))
        {
            return;
        }

        if (strchr(d.address, '|') == NULL)
        {
            break;
        }

        showWarning("Address cannot contain '|'. Please try again.");
    }

    /* Generate user ID */
    d.id = getNextUserId();

    if (d.id == -1)
    {
        showError("Registration failed. Cannot generate user ID.");
        pressEnterToContinue();
        return;
    }

    d.role = ROLE_CUSTOMER;
    d.isActive = 1;

    /* Save customer */
    FILE *fp = fopen(USERS_FILE, "a");

    if (fp == NULL)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "Cannot open file: %s", USERS_FILE);
        showError(msg);
        pressEnterToContinue();
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
        showError("Error saving registration data.");
        pressEnterToContinue();
        return;
    }

    if (fclose(fp) != 0)
    {
        showError("Error closing users file.");
        pressEnterToContinue();
        return;
    }

    drawFooter();
    char successMsg[80];
    snprintf(successMsg, sizeof(successMsg), "Customer registered successfully! ID = %d", d.id);
    showSuccess(successMsg);
    pressEnterToContinue();
}
/* Update profile */
void updateprofile(char logginuser[])
{
    logginuser[strcspn(logginuser, "\r\n")] = '\0';

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "Cannot open file: %s", USERS_FILE);
        showError(msg);
        pressEnterToContinue();
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL)
    {
        fclose(fp);
        showError("Cannot create temporary file.");
        pressEnterToContinue();
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

        clearScreen();
        drawHeader("UPDATE PROFILE");
        printf("\n  " DIM "Current profile for " RESET BOLD WHITE "%s\n\n" RESET, logginuser);
        printf("  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
        printf("  " MAGENTA "│" RESET " Email   : " CYAN "%-28.28s " RESET MAGENTA "│\n" RESET, d.email);
        printf("  " MAGENTA "│" RESET " Phone   : " CYAN "%-28.28s " RESET MAGENTA "│\n" RESET, d.tpNumber);
        printf("  " MAGENTA "│" RESET " Address : " CYAN "%-28.28s " RESET MAGENTA "│\n" RESET, d.address);
        printf("  " MAGENTA "╰──────────────────────────────────────────╯\n\n" RESET);

        printf("  " BOLD WHITE "Enter New Details:\n\n" RESET);

        /* Email validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "  " CYAN "❯" RESET " New Email: ", d.email, sizeof(d.email)))
            {
                error = 1;
                break;
            }
            char *at = strchr(d.email, '@');
            if (at != NULL && at != d.email && strchr(at + 1, '.') != NULL && strchr(d.email, '|') == NULL)
            {
                break;
            }
            showWarning("Invalid email address. Please try again.");
        }
        if (error) break;

        /* Phone validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "  " CYAN "❯" RESET " New Phone Number (10 digits): ",
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
            if (validPhone) break;
            showWarning("Phone number must contain exactly 10 digits.");
        }
        if (error) break;

        /* Address validation */
        while (1)
        {
            if (!readNonEmptyString(
                    "  " CYAN "❯" RESET " New Address: ", d.address, sizeof(d.address)))
            {
                error = 1;
                break;
            }
            if (strchr(d.address, '|') == NULL) break;
            showWarning("Address cannot contain '|'. Please try again.");
        }
        if (error) break;

        if (fprintf(temp, "%s|%s|%s|%s|%s|%s|%s|%s\n",
                    id, d.username, d.password, d.email, d.tpNumber, d.address, role, active) < 0)
        {
            error = 1;
            break;
        }
        found = 1;
    }

    if (ferror(fp)) error = 1;
    if (fclose(fp) != 0) error = 1;
    if (fclose(temp) != 0) error = 1;

    if (error)
    {
        remove("temp.txt");
        showError("Profile update failed. Original data was preserved.");
        pressEnterToContinue();
        return;
    }

    if (!found)
    {
        remove("temp.txt");
        char msg[80];
        snprintf(msg, sizeof(msg), "User not found: %s", logginuser);
        showError(msg);
        pressEnterToContinue();
        return;
    }

    if (rename("temp.txt", USERS_FILE) != 0)
    {
        showError("Could not replace the users file. Temp file retained: temp.txt");
        pressEnterToContinue();
        return;
    }

    showSuccess("Profile updated successfully!");
    pressEnterToContinue();
}
/* Logout */
void logout(char logginuser[])
{
    logginuser[0] = '\0';
    clearScreen();
    drawHeader("LOGOUT");
    printf("\n");
    showSuccess("Logout successful! See you again.");
    printf("\n  " DIM "Returning to main menu...\n" RESET);
    pressEnterToContinue();
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

