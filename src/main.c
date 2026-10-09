#include <stdio.h>
#include <stdlib.h>

/* Function declarations */
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

        while (getchar() != '\n')
        {
            /* Clear invalid input */
        }

        continue;
    }

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
            printf("\nThank you for using "
                   "E-Commerce Management System!\n");
            break;

        default:
            printf("\nInvalid choice! "
                   "Please select 1-4.\n");
    }

} while (choice != 4);

return 0;

}

void displayMainMenu(void)
{
printf("\n");
printf("========================================\n");
printf("      E-COMMERCE MANAGEMENT SYSTEM\n");
printf("========================================\n");
printf("1. Customer Login\n");
printf("2. Customer Registration\n");
printf("3. Admin Login\n");
printf("4. Exit\n");
}
void customerLogin(void)
{
printf("\nCustomer Login module is not ready yet.\n");
}

void customerRegistration(void)
{
printf("\nCustomer Registration module is not ready yet.\n");
}

void adminLogin(void)
{
printf("\nAdmin Login module is not ready yet.\n");
}
