#include <stdio.h>
#include <stdlib.h>
#include "ui.h"

void clearScreen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


void showSplash() {
    clearScreen();
    printf("\x1b[1m\x1b[36m");
    printf("  NORDIC E-COMMERCE\n");
    printf("\x1b[0m\n");
}
#include <stdio.h>

void drawHeader(const char *title)
{
    printf("\n\x1b[36m┌────────────────────────────────────────┐\x1b[0m\n");
    printf("\x1b[36m│\x1b[0m  \x1b[1m%-35.35s\x1b[0m   \x1b[36m│\x1b[0m\n", title);
    printf("\x1b[36m└────────────────────────────────────────┘\x1b[0m\n");
}



void drawMenuItem(int num, const char* title, const char* desc) {
    if(num == 99) {
        printf("\x1b[36m│\x1b[0m  \x1b[33m%d. %-15s \x1b[36m\x1b[0m\n", num, title);
    } else {
        printf("\x1b[36m│\x1b[0m  \x1b[32m%d. %-15s \x1b[2m%s \x1b[0m\x1b[36m\x1b[0m\n", num, title, desc);
    }
}

void drawFooter() {
    printf("\x1b[36m└────────────────────────────────────────┘\x1b[0m\n");
    printf("\n\x1b[1m> Enter choice: \x1b[0m");
}

void displayMainMenu(void) {
    clearScreen();
    drawHeader("Main Menu");
    drawMenuItem(1, "Login", "• Access account");
    drawMenuItem(2, "Registration", "• Create new");
    drawMenuItem(3, "View products", "• Browse");
    drawMenuItem(4, "Instructions", "• How to use");
    drawMenuItem(5, "About Us", "• Our story");
    drawMenuItem(99, "Administrative", "");
    drawFooter();
}

void displayCustomerMenu(void) {
    clearScreen();
    drawHeader("Customer Menu");
    drawMenuItem(1, "View All Products", "• Browse");
    drawMenuItem(2, "Search of Products", "• Find");
    drawMenuItem(3, "My Cart", "• 2 items");
    drawMenuItem(4, "My Order", "• History");
    drawMenuItem(5, "Logout", "• Exit");
    drawFooter();
}

void displayAdminMenu(void) {
    clearScreen();
    drawHeader("Admin Menu");
    drawMenuItem(1, "Add New Product", "• Create");
    drawMenuItem(2, "Update Products", "• Edit");
    drawMenuItem(3, "Delete Products", "• Remove");
    drawMenuItem(4, "View All Users", "• Manage");
    drawMenuItem(5, "View All Orders", "• Track");
    drawMenuItem(6, "Dashboard", "• Stats");
    drawMenuItem(7, "Logout", "• Exit admin");
    drawFooter();
}
// void displayProductMenu(void) {
//     clearScreen();
//     drawHeader("Admin Menu");
//     drawMenuItem(1, "Add New Product", "• Create");
//     drawMenuItem(2, "Update Products", "• Edit");
//     drawMenuItem(3, "Delete Products", "• Remove");
//     drawMenuItem(4, "View All Users", "• Manage");
//     drawMenuItem(5, "View All Orders", "• Track");
//     drawMenuItem(6, "Dashboard", "• Stats");
//     drawMenuItem(7, "Logout", "• Exit admin");
//     drawFooter();
// }