#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "users.h"
#include "products.h"
#include "admin.h"
#include "ui.h"
#include "cart.h"
#include "orders.h"

void customerLogin(void);
void customerRegistration(void);
void adminLogin(void);
void showInstructions(void);
void showAboutUs(void);

int main(void)
{
    int choice;
    showSplash(); // display splash screen
    printf("\nPress Enter to continue...");
    getchar(); // get input

    do
    {
        clearScreen();
        displayMainMenu();
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            printf("\n%sInvalid input! Please enter a number.%s\n", YELLOW, RESET);
            while (getchar() != '\n')
                ;
            continue;
        }
        while (getchar() != '\n')
            ;

        switch (choice)
        {
        case 1:
            customerLogin();
            break;
        case 2:
            customerRegistration();
            break;
        case 3:
            viewProducts(); // Guest view
            printf("\nPress Enter...");
            getchar();
            break;
        case 4:
            showInstructions();
            break;
        case 5:
            showAboutUs();
            break;
        case 99:
            adminLogin();
            break;
        case 0:
            clearScreen();
            printf("\n%sThank you for using E-Commerce Management System!%s\n", GREEN, RESET);
            break;
        default:
            printf("\n%sInvalid choice! Please select 0-5, 99%s\n", YELLOW, RESET);
            printf("Press Enter...");
            getchar();
        }
    } while (choice != 0);
    return 0;
}

void customerLogin(void)
{
    char user[50] = "";
    if (login(user) == 1)
    {
        // Trim username
        user[strcspn(user, "\r\n")] = 0;
        int i = 0;
        while (user[i] == ' ' || user[i] == '\t')
            i++;
        if (i > 0)
            memmove(user, user + i, strlen(user + i) + 1);
        int len = strlen(user);
        while (len > 0 && (user[len - 1] == ' ' || user[len - 1] == '\t'))
            user[--len] = 0;

        printf("\n%sCustomer %s logged in successfully!%s\n", GREEN, user, RESET);
        printf("Press Enter...");
        getchar();

        int userIn;
        while (1)
        {
            clearScreen();
            displayCustomerMenu();
            printf("\nEnter choice: ");
            if (scanf("%d", &userIn) != 1)
            {
                while (getchar() != '\n')
                    ;
                continue;
            }
            while (getchar() != '\n')
                ;

            switch (userIn)
            {
            case 1:
                viewProductsAndAddToCart(user);
                printf("\nPress Enter to continue...");
                getchar();
                break;
            case 2:
                searchProduct();
                printf("\nPress Enter to continue...");
                getchar();
                break;
            case 3:
                // viewCart(user); // MAIN FEATURE
                viewCart(user);
                break;
            case 4:
                myOrders(user);
                printf("\nPress Enter to continue...");
                getchar();
                break;
            case 5:
                return;
            default:
                printf("Invalid choice!\n");
                printf("Press Enter...");
                getchar();
                break;
            }
        }
    }
}

void customerRegistration(void)
{
    clearScreen();
    registerCustomer();
    printf("\nPress Enter to go back...");
    getchar();
}

void adminLogin(void)
{
    char user[50];
    clearScreen();
    drawHeader("ADMIN LOGIN");
    if (login(user) == 1)
    {
        int role = getUserRole(user);
        if (role != 0)
        {
            printf("\n%sAdmin %s logged in%s\n", GREEN, user, RESET);
            printf("Press Enter...");
            getchar();

            int adminChoice;
            while (1)
            {
                displayAdminMenu();
                printf("\nEnter choice: ");
                if (scanf("%d", &adminChoice) != 1)
                {
                    while (getchar() != '\n')
                        ;
                    continue;
                }
                while (getchar() != '\n')
                    ;

                if (adminChoice == 7 || adminChoice == 0)
                    break;

                // Call product admin functions
                switch (adminChoice)
                {
                case 1:
                    addProduct();
                    break;
                case 2:
                    updateProduct();
                    break;
                case 3:
                    deleteProduct();
                    break;
                // case 4:  break; //create view all users
                case 5:
                    updateOrderStatus();
                    break;
                case 7:
                    logout(user);
                    break;
                default:
                    printf("Module %d\n", adminChoice);
                }
                printf("\nPress Enter...");
                getchar();
            }
        }
    }
}

void showInstructions(void)
{
    clearScreen();
    drawHeader("INSTRUCTIONS");
    printf("\n  1. Use numbers to navigate\n");
    printf("  2. Login to buy products\n");
    printf("  3. Cart -> Orders flow\n");
    printf("  4. Admin: 99\n");
    printf("\nPress Enter to go back...");
    getchar();
}

void showAboutUs(void)
{
    clearScreen();
    drawHeader("ABOUT US");
    printf("\n  Nordic E-Commerce v1.0\n");
    printf("  Built on Ubuntu - C Project\n");
    printf("  Main Features: Login, Products, Cart, Orders\n");
    printf("\nPress Enter to go back...");
    getchar();
}