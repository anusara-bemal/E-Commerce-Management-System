#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "users.h"
#include "products.h"
#include "cart.h"
#include "orders.h"
#include "admin.h"

void displayMainMenu(void);
void customerLogin(void);
void customerRegistration(void);
void adminLogin(void);

int main(void)
{
    int choice;
    do
    {
        displayMainMenu();
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice)!= 1)
        {
            printf("\nInvalid input! Please enter a number.\n");
            while (getchar()!= '\n');
            continue;
        }
        while (getchar()!= '\n'); // scanf \n clear

        switch (choice)
        {
            case 1:
                customerLogin();
                break;
            case 2:
                customerRegistration();
                break;
            case 3:
                adminLogin();
                break;
            case 4:
                printf("\nThank you for using E-Commerce Management System!\n");
                break;
            default:
                printf("\nInvalid choice! Please select 1-4.\n");
        }
    } while (choice!= 4);
    return 0;
}

void displayMainMenu(void)
{
    printf("\n========================================\n");
    printf(" E-COMMERCE MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf("1. Customer Login\n");
    printf("2. Customer Registration\n");
    printf("3. Admin Login\n");
    printf("4. Exit\n");
}
void customerLogin(void)
{
    char user[50] = "";
    if(login(user) == 1){
        printf("\nCustomer %s logged in successfully!\n", user);

        // --- මේ ටික තමයි fix එක ---
        // users.c එකේ bug එක: \r නිසා User Not Found එනවා. අපි main එකෙන්ම clean කරනවා
        user[strcspn(user, "\r\n")] = 0;
        // ඉස්සරහ පිටිපස space තියෙනවා නම් අයින් කරනවා
        int i=0; while(user[i]==' ' || user[i]=='\t') i++;
        if(i>0) memmove(user, user+i, strlen(user+i)+1);
        int len = strlen(user);
        while(len>0 && (user[len-1]==' ' || user[len-1]=='\t')) user[--len]=0;

        int userIn;
        printf("\n1. Update profile\n");
        printf("2. Logout\n");
        printf("Enter choice: ");
        scanf("%d",&userIn);
        while(getchar()!='\n');

        switch (userIn)
        {
        case 1:
            updateprofile(user); // දැන් clean කරපු user නම යන නිසා User Not Found එන්නේ නෑ
            break;
        case 2:
            logout(user);
            break;
        default:
            printf("Invalid choice!\n");
            break;
        }
    }
}
void customerRegistration(void)
{
    registerCustomer();
}

void adminLogin(void)
{
    char user[50];
    printf("\n--- Admin Login ---\n");
    if(login(user) == 1){
        printf("Admin %s logged in\n", user);
        // TODO: check role == ROLE_ADMIN
        // showAdminMenu();
    }
}