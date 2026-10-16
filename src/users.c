#include <stdio.h>
#include <string.h>
#include "models.h"
#include "users.h"

// id eka auto hadanawa
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

int login(char logginuser[])
{
    char intuser[MAX_USERNAME];
    char intpass[MAX_PASSWORD];

    printf("==================================================\n");
    printf("CUSTOMER LOGIN\n");
    printf("==================================================\n");

    printf("Enter User Name : ");
    fgets(intuser, sizeof(intuser), stdin);
    intuser[strcspn(intuser, "\n")] = 0;

    printf("Enter Password : ");
    fgets(intpass, sizeof(intpass), stdin);
    intpass[strcspn(intpass, "\n")] = 0;

    FILE *fp = fopen(USERS_FILE, "r");
    if (fp == NULL) {
        printf("Can't Open File: %s\n", USERS_FILE);
        return 0;
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

        p = strtok(NULL, "|"); // username
        if(p==NULL) continue;
        strcpy(f_user, p);

        p = strtok(NULL, "|"); // password
        if(p==NULL) continue;
        strcpy(f_pass, p);
         

        // save logging use
        if(strcmp(f_user, intuser)==0 && strcmp(f_pass, intpass)==0)
        {
            strcpy(logginuser, intuser); 
            found=1;
            break;
        }
    }
    fclose(fp);

    if(found == 0){
        printf("\nYou have no Account Please Register.\n");
        return 0;
    } else {
        printf("\nlogin successfully!\n");
        return 1;
    }
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

    printf("==================================================\n");
    printf("CUSTOMER REGISTRATION\n");
    printf("==================================================\n");

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

    if(strchr(d.email,'@')== NULL)
    {
        printf("Invalid email\n");
        return;
    }

    //mobile number and address
    
    printf("Phone Number : ");
    fgets( d.tpNumber,sizeof( d.tpNumber),stdin);
    d.tpNumber[strcspn(d.tpNumber, "\n")] = 0;
    //check phone number valid
   if (strlen(d.tpNumber) != 10)
   {
     printf("Invalid Phone Number! Enter 10 digits.\n");
     return;
    }
 
   //address
   printf("Address : ");
   fgets( d.address,sizeof( d.address),stdin);
   d.address[strcspn( d.address,"\n")]=0;

    d.id = getNextUserId();
    d.role = ROLE_CUSTOMER;
    d.isActive = 1;

    FILE *fp = fopen(USERS_FILE,"a");
    if (fp == NULL) {
        printf("Can't Open File\n");
        return;
    }

    fprintf(fp, "%d|%s|%s|%s|%s|%s|%d|%d\n",
        d.id, d.username, d.password, d.email,
        d.tpNumber, d.address, d.role, d.isActive);
    fclose(fp);

    printf("\n%s Customer Registration Successfully.... ID = %d\n", d.username, d.id);
}


//update profile
void updateprofile(char logginuser[])
{
    FILE *fp=fopen(USERS_FILE,"r");

    if(fp==NULL)
    {
        printf("Can't Open File\n");
        return;
    }

    FILE *temp=fopen("temp.txt","w");

    if(temp==NULL)
    {
        fclose(fp);
        printf("Cant't Create Temporary File\n");
        return;
    }

    char line[500];
    int found=0;

    while(fgets(line,sizeof(line),fp))
    {
        line[strcspn(line,"\n")]=0;

        char copy[500];
        strcpy(copy,line);

        char *id=strtok(copy, "|");
        char *username=strtok(NULL,"|");
        char *password = strtok(NULL, "|");
        char *email = strtok(NULL, "|");
        char *phone = strtok(NULL, "|");
        char *address = strtok(NULL, "|");
        char *role = strtok(NULL, "|");
        char *active = strtok(NULL, "|");

        if(username != NULL && strcmp(username, logginuser)==0 && password != NULL &&email != NULL && phone != NULL && address !=NULL && role != NULL && active != NULL)
        {
            User d;
            strcpy(d.username, username);
            strcpy(d.password, password);
            strcpy(d.email, email);
            strcpy(d.tpNumber, phone);
            strcpy(d.address, address);

            printf("New Email :");
            fgets(d.email,sizeof(d.email),stdin);
            d.email[strcspn(d.email, "\n")]=0;

            if(strchr(d.email, '@')==NULL)
            {
                 printf("Invalid Email\n");

                fclose(fp);
                fclose(temp);
                remove("temp.txt");
                return;
            }

            printf("New Phone Number: ");
            fgets(d.tpNumber, sizeof(d.tpNumber), stdin);
            d.tpNumber[strcspn(d.tpNumber, "\n")] = 0;

            if (strlen(d.tpNumber) != 10)
            {
                printf("Phone Number Must Have 10 Digits\n");

                fclose(fp);
                fclose(temp);
                remove("temp.txt");
                return;
            }
            printf("New Address: ");
            fgets(d.address, sizeof(d.address), stdin);
            d.address[strcspn(d.address, "\n")] = 0;

            fprintf(temp, "%s|%s|%s|%s|%s|%s|%s|%s\n",
                    id, d.username, d.password, d.email,
                    d.tpNumber, d.address, role, active);

            found = 1;
        }
        else
        {
            fprintf(temp, "%s\n", line);
        }
        }

        fclose(fp);
        fclose(temp);


        if (found)
    {
        if (remove(USERS_FILE) != 0)
        {
            printf("Error Removing Old File\n");
            return;
        }

        if (rename("temp.txt", USERS_FILE) != 0)
        {
            printf("Error Updating Profile File\n");
            return;
        }

        printf("Profile Updated Successfully!\n");
    }
    else
    {
        remove("temp.txt");
        printf("User Not Found!\n");
    }
}



//logout
void logout(char logginuser[])
{
    
    strcpy(logginuser,"");
    printf("Logout Successfully\n");

}
