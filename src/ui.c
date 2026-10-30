#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"

/* Clear terminal screen */
void clearScreen(void)
{
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
}

/* Display application branding */
void showSplash(void)
{
clearScreen();


printf("\n");
printf(CYAN BOLD);
printf("  ============================================================\n");
printf("   _   _  ___  ____  ____ ___ ____\n");
printf("  | \\ | |/ _ \\|  _ \\|  _ \\_ _/ ___|\n");
printf("  |  \\| | | | | |_) | | | | |\\___ \\\n");
printf("  | |\\  | |_| |  _ <| |_| | | ___) |\n");
printf("  |_| \\_|\\___/|_| \\_\\____/___|____/\n");
printf("\n");
printf("                 E-COMMERCE MANAGEMENT SYSTEM\n");
printf(RESET);

printf(CYAN);
printf("  ============================================================\n");
printf(RESET);

printf("\n");
printf(DIM "  Simple. Reliable. Efficient.\n" RESET);
printf("\n");


}

/* Draw a consistent page header */
void drawHeader(const char *title)
{
printf("\n");
printf(CYAN);
printf("  +──────────────────────────────────────────────────────────+\n");
printf("  |" RESET BOLD WHITE "  NORDIC E-COMMERCE                                       " RESET CYAN "|\n");
printf("  +__________________________________________________________+\n");
printf("  |" RESET BOLD "  %-56.56s" RESET CYAN "|\n", title);
printf("  +──────────────────────────────────────────────────────────+\n");
printf(RESET);
}

/* Draw one menu option */
void drawMenuItem(int num, const char *title, const char *desc)
{
printf(CYAN "  | " RESET);


if (num == 99)
    printf(YELLOW BOLD "%-3d" RESET, num);
else
    printf(GREEN BOLD "%-3d" RESET, num);

printf(" " WHITE "%-25.25s" RESET, title);
printf(DIM "%-18.18s" RESET, desc);

printf(CYAN " |\n" RESET);


}

/* Draw menu footer */
void drawFooter(void)
{
printf(CYAN "  +──────────────────────────────────────────────────────────+\n" RESET);
printf("\n");
printf(BOLD CYAN "  >> " RESET "Enter your choice: ");
fflush(stdout);
}

/* Feedback messages */
void showSuccess(const char *message)
{
printf("\n" GREEN BOLD "  [SUCCESS] " RESET "%s\n", message);
}

void showError(const char *message)
{
printf("\n" RED BOLD "  [ERROR] " RESET "%s\n", message);
}

void showWarning(const char *message)
{
printf("\n" YELLOW BOLD "  [WARNING] " RESET "%s\n", message);
}

/* Pause before returning to a menu */
void pressEnterToContinue(void)
{
char buffer[8];


printf("\n" DIM "  Press Enter to continue..." RESET);
fflush(stdout);

if (fgets(buffer, sizeof(buffer), stdin) != NULL)
{
    if (strchr(buffer, '\n') == NULL)
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
}


}

/* Main menu */
void displayMainMenu(void)
{
clearScreen();
drawHeader("MAIN MENU");


drawMenuItem(1, "Login", "Access account");
drawMenuItem(2, "Registration", "Create account");
drawMenuItem(3, "View Products", "Browse catalog");
drawMenuItem(4, "Instructions", "Help");
drawMenuItem(5, "About Us", "Application info");
drawMenuItem(99, "Administration", "Admin access");
drawMenuItem(0, "Exit", "Close application");

drawFooter();


}

/* Customer menu */
void displayCustomerMenu(void)
{
clearScreen();
drawHeader("CUSTOMER DASHBOARD");


drawMenuItem(1, "View Products", "Browse catalog");
drawMenuItem(2, "Search Products", "Find products");
drawMenuItem(3, "My Cart", "Manage cart");
drawMenuItem(4, "My Orders", "Order history");
drawMenuItem(5, "Logout", "End session");

drawFooter();


}

/* Admin menu */
void displayAdminMenu(void)
{
clearScreen();
drawHeader("ADMIN DASHBOARD");


drawMenuItem(1, "Add Product", "Create product");
drawMenuItem(2, "Update Product", "Edit details");
drawMenuItem(3, "Delete Product", "Remove product");
drawMenuItem(4, "View Users", "User management");
drawMenuItem(5, "Manage Orders", "Order status");
drawMenuItem(6, "Dashboard", "System overview");
drawMenuItem(7, "Logout", "End session");

drawFooter();


}
