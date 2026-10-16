#include <stdio.h>
#include <string.h>
#include "models.h"
#include "users.h"

// ===== PRIVATE HELPER - මේක උඩම තියෙන්න ඕන =====
static int getNextUserId() {
    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) return 1;

    char line[300];
    int lastId = 0;
    while (fgets(line, sizeof(line), fp)) {
        int id;
        if (sscanf(line, "%d|", &id) == 1) {
            if (id > lastId) lastId = id;
        }
    }
    fclose(fp);
    return lastId + 1;
}

void login()
{
    char intuser[MAX_USERNAME];
    char intpass[MAX_PASSWORD];

    printf("========================================\n");
    printf(" CUSTOMER LOGIN\n");
    printf("========================================\n");

    printf("Enter User Name : ");
    fgets(intuser, sizeof(intuser), stdin);
    intuser[strcspn(intuser, "\n")] = 0;

    printf("Enter Password : ");
    fgets(intpass, sizeof(intpass), stdin);
    intpass[strcspn(intpass, "\n")] = 0;

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) {
        printf("Can't Open File: %s\n", USERS_FILE);
        return;
    }

    char line[300];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = 0;
        char *p = strtok(line, "|");
        if(p == NULL) continue;

        char f_user[MAX_USERNAME];
        char f_pass[MAX_PASSWORD];

        p = strtok(NULL, "|");
        if(p==NULL) continue;
        strcpy(f_user, p);

        p = strtok(NULL, "|");
        if(p==NULL) continue;
        strcpy(f_pass, p);

        if(strcmp(f_user, intuser)==0 && strcmp(f_pass, intpass)==0)
        {
            found=1;
            break;
        }
    }

    if(found == 0) printf("\nYou have no Account Please Register.\n");
    else printf("\nlogin successfully!\n");

    fclose(fp);
}

int validuser(char username[])
{
    if(strlen(username)<3)
    {
        printf("Username must have at least 3 characters...\n");
        return 0;
    }

    FILE *fp = fopen(USERS_FILE, "r");
    if(fp == NULL) return 1;

    char line[300];
    while(fgets(line,sizeof(line),fp))
    {
        line[strcspn(line,"\n")]=0;
        char temp[300];
        strcpy(temp, line);
        strtok(temp, "|");
        char *p = strtok(NULL,"|");

        if(p!=NULL && strcmp(p, username)==0)
        {
            fclose(fp);
            printf("Username already exists!\n");
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

void registerCustomer()
{
    User d;

    printf("========================================\n");
    printf(" CUSTOMER REGISTRATION\n");
    printf("========================================\n");

    printf("User Name : ");
    fgets(d.username,sizeof(d.username),stdin);
    d.username[strcspn(d.username,"\n")]=0;
    if(validuser(d.username)==0) return;

    printf("Password : ");
    fgets(d.password,sizeof(d.password),stdin);
    d.password[strcspn(d.password,"\n")]=0;

    printf("Email : ");
    fgets(d.email,sizeof(d.email), stdin);
    d.email[strcspn(d.email,"\n")]=0;

    d.id = getNextUserId();
    d.role = ROLE_CUSTOMER;
    d.isActive = 1;

    FILE *fp = fopen(USERS_FILE,"a");
    if (fp == NULL) {
        printf("Can't Open File\n");
        return;
    }

    fprintf(fp, "%d|%s|%s|%s|%d|%d\n", d.id, d.username, d.password, d.email, d.role, d.isActive);
    fclose(fp);

    printf("\n%s Customer Registration Successfully.... ID = %d\n", d.username, d.id);
}


//user id eka imcrement unata same user id eka txt eke save wenne
