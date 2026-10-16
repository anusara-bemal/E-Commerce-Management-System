#include <stdio.h>
#include <stdlib.h>
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
    char user[50];
    if(login(user) == 1){
        printf("\nCustomer %s logged in successfully!\n", user);
        // TODO: showCustomerMenu(user);
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