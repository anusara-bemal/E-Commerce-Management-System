#include <stdio.h>
#include <stdlib.h>
#include "models.h"
#include "users.h"
#include "products.h"
#include "cart.h"
#include "orders.h"
#include "admin.h"

void displayMainMenu(void);
void customerRegistration(void);
void customerLogin(void);
void adminLogin(void);

int main(void)
{
    int choice;
    do
    {
        displayMainMenu();
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input! Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // මේ line එක අනිවා ඕන - scanf එකේ \n එක මකන්න

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
    } while (choice != 4);
    return 0;
}

void displayMainMenu(void)
{
    printf("\n========================================\n");
    printf("      E-COMMERCE MANAGEMENT SYSTEM\n");
    printf("========================================\n");
    printf("1. Customer Login\n");
    printf("2. Customer Registration\n");
    printf("3. Admin Login\n");
    printf("4. Exit\n");
}

void customerLogin(void)
{
    // අපි හදපු users.c එකේ login() එක call කරනවා
    login();
    
    // මෙතනට පස්සේ customer dashboard එකට යන්න පුළුවන්
    // if(login success) -> showCustomerMenu()
}

void customerRegistration(void)
{
    // අපි හදපු users.c එකේ registerCustomer() එක call කරනවා
    registerCustomer();
}

void adminLogin(void)
{
    printf("\n--- Admin Login ---\n");
    // admin login එකත් customer login එකමයි, role එක විතරක් වෙනස්
    // දැන්ට අපි customer login එකම call කරමු
    login(); 
    // පස්සේ role == ROLE_ADMIN ද කියලා check කරන්න
}