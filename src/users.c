#include<stdio.h>
#include<string.h>

struct costomer{
    char name[50];
    char username[30];
    char email[30];
    char password[10];
    char number[15];
    char address[50];
};

void login()
{
 struct costomer d;
 char intuser[30];
 char intpass[10];

 printf("==================================================\nCUSTOMER LOGIN\n==================================================\n");

 printf("Enter User Name : ");
 fgets(intuser,sizeof(intuser),stdin);
 intuser[strcspn(intuser,"\n")]=0;

 printf("Enter Password : ");
 fgets(intpass,sizeof(intpass),stdin);
 intpass[strcspn(intpass,"\n")]=0;

 FILE *fp = fopen("coustemer.txt","r");
 
 if (fp == NULL) {
        printf("Can't Open File\n");
        return;
    }

    char line[200];
    int found=0;
    char *p;
    char f_user[30], f_pass[10];


while (fgets(line,sizeof(line),fp))
{
    line[strcspn(line, "\n")] = 0;

    
   
   p = strtok(line, ","); 
        p = strtok(NULL, ","); 
        if(p==NULL) {
           continue;
        }
        strcpy(f_user, p);
        

        p = strtok(NULL, ","); 
        if(p==NULL) {
            continue;
        }
        strcpy(f_pass, p);

   if(strcmp(f_user, intuser)==0 && strcmp(f_pass, intpass)==0)
    {
        
        found=1;
        break;
    }
}
    if(found == 0)
    {
        printf("\nYou have no Account Please Register.\n");
        
    }
    else{
        printf("\nlogin sucsessfully!\n");
    }

fclose(fp);

 
}

//user name Validation & check exsists
int validuser(char username[])
{
    if(strlen(username)<3)
    {
        printf("Username must have at least 3 characters...\n");
        return 0;
    }

    FILE *fp = open("coustemer.txt" , "r");
    if(fp == NULL)
    {
        return 1;
    }
    char line[200];
    char *p;
    while(fgets(line,sizeof(line),fp))
    {
        line[strcspn(line,"\n")]=0;
        p=strtok(line, ",");
        p=strtok(NULL,",");

        if(p !=NULL && strcmp(p, username)==0)
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
 struct costomer d;

 printf("==================================================\nCUSTOMER REGISTRATION\n==================================================\n");

 printf("Name : ");
 fgets(d.name,sizeof(d.name),stdin);
 d.name[strcspn(d.name,"\n")]=0;

 printf("User Name : ");
 fgets(d.username,sizeof(d.username),stdin);
 d.username[strcspn(d.username,"\n")]=0;
 if(validuser(d.username)==0)
 {
    printf("Please try another username.\n");
    return;
 }

 printf("Password : ");
 fgets(d.password,sizeof(d.password),stdin);
 d.password[strcspn(d.password,"\n")]=0;

 printf("Email : ");
 fgets(d.email,sizeof(d.email), stdin);
 d.email[strcspn(d.email,"\n")]=0;

 printf("Phone Number : ");
 fgets( d.number,sizeof( d.number),stdin);
 d.number[strcspn( d.number,"\n")]=0;

 printf("Address : ");
 fgets( d.address,sizeof( d.address),stdin);
 d.address[strcspn( d.address,"\n")]=0;

 //save 
 FILE *fp = fopen("coustemer.txt","a");

 if (fp == NULL) {
        printf("Can't Open File\n");
        return;
    }

 fprintf(fp, "%s,%s,%s,%s,%s,%s\n",d.name,d.username,d.password,d.email,d.number,d.address);
 fclose(fp);

 printf("\n\n%s Customer Registration Sucssesfully............\n", d.name);
}



int main()
{
 login(); 
 registerCustomer();
   
 
return 0;
}
