#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/ui.h"

void clearScreen(void){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void showSplash(void){
    clearScreen();
    printf("\n");
    printf(MAGENTA BOLD);
    printf("  ╭───────────────────────────────────────────────────╮\n");
    printf("  │   ███████╗      ███████╗██╗  ██╗ ██████╗ ██████╗  │\n");
    printf("  │   ██╔════╝      ██╔════╝██║  ██║██╔═══██╗██╔══██╗ │\n");
    printf("  │   ███████╗      ███████╗███████║██║   ██║██████╔╝ │\n");
    printf("  │   ╚════██║      ╚════██║██╔══██║██║   ██║██╔═══╝  │\n");
    printf("  │   ███████║      ███████║██║  ██║╚██████╔╝██║      │\n");
    printf("  ╰───────────────────────────────────────────────────╯\n" RESET);
    printf(CYAN "  ╭─ System ────────────────────────────────────╮\n");
         printf("  │  PROJECT E-Commerce Management System v1.0  │\n");
         printf("  ╰─────────────────────────────────────────────╯\n" RESET);
    printf("\n   ● ● ● ● ● ● ● ●\n\n");
}

void drawHeader(const char* title){
    printf("\n");
    printf("%s%s", MAGENTA, R_TL);
    for(int i=0;i<52;i++) printf("%s", R_H);
    printf("%s\n", R_TR);
    printf("%s%s  %s%-48s  %s%s\n", MAGENTA, R_V, BOLD WHITE, title, RESET MAGENTA, R_V);
    printf("%s", R_BL);
    for(int i=0;i<52;i++) printf("%s", R_H);
    printf("%s\n" RESET, R_BR);
}

void drawFooter(void){
    printf("%s%s", MAGENTA, R_BL);
    for(int i=0;i<52;i++) printf("%s", R_H);
    printf("%s\n" RESET, R_BR);
}

void drawMenuItem(int num, const char* text, const char* desc){
    printf("%s%s  " RESET GREEN BOLD "%d. " RESET WHITE "%-20s " RESET DIM "%s\n" RESET, MAGENTA, R_V, num, text, desc ? desc : "");
}

void displayMainMenu(void){
    drawHeader("MAIN MENU");
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawMenuItem(1, "Customer Login", "");
    drawMenuItem(2, "Customer Registration", "");
    drawMenuItem(3, "View Products", "Guest");
    drawMenuItem(4, "Instructions", "");
    drawMenuItem(5, "About Us", "");
    printf("%s%s\n" RESET, MAGENTA, R_V);
    printf("%s%s  %s99. Admin Login\n" RESET, MAGENTA, R_V, DIM);
    printf("%s%s  %s0. Exit\n" RESET, MAGENTA, R_V, DIM);
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawFooter();
}

void displayAdminMenu(void){
    clearScreen();
    drawHeader("ADMIN PANEL");
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawMenuItem(1, "Add Product", "");
    drawMenuItem(2, "Update Product", "");
    drawMenuItem(3, "Delete Product", "");
    drawMenuItem(4, "User Management", "");
    drawMenuItem(5, "Update Order Status", "");
    drawMenuItem(6, "Dashboard", "");
    drawMenuItem(7, "Logout", "");
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawFooter();
}

void displayCustomerMenu(const char* username){
    clearScreen();
    char title[80];
    snprintf(title, sizeof(title), "WELCOME %s", username);
    drawHeader(title);
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawMenuItem(1, "View Products & Buy", "");
    drawMenuItem(2, "Search Product", "");
    drawMenuItem(3, "My Cart", "");
    drawMenuItem(4, "My Orders", "");
    drawMenuItem(5, "Logout", "");
    printf("%s%s\n" RESET, MAGENTA, R_V);
    drawFooter();
}

void showSuccess(const char* msg){ printf("\n" GREEN "  [SUCCESS] %s\n" RESET, msg); }
void showError(const char* msg){ printf("\n" RED "  [ERROR] %s\n" RESET, msg); }
void showWarning(const char* msg){ printf("\n" YELLOW "  [WARNING] %s\n" RESET, msg); }

void pressEnterToContinue(void){
    printf(DIM "\n  Press Enter to continue..." RESET);
    int c;
    while((c = getchar()) != '\n' && c != EOF);
    getchar();
}
#define TABLE_COL_WIDTH 14

void printTableHeader(int columns, ...)
{
    va_list args;
    va_start(args, columns);

    printf("\n" MAGENTA "  ");

    for (int i = 0; i < columns; i++)
    {
        const char *heading = va_arg(args, const char *);

        printf(BOLD WHITE "%-*.*s" RESET MAGENTA,
               TABLE_COL_WIDTH,
               TABLE_COL_WIDTH,
               heading ? heading : "");

        if (i < columns - 1)
            printf(" | ");
    }

    printf("\n");

    va_end(args);
}

void printTableDivider(int columns)
{
    printf(MAGENTA "  ");

    for (int i = 0; i < columns; i++)
    {
        for (int j = 0; j < TABLE_COL_WIDTH; j++)
            printf("─");

        if (i < columns - 1)
            printf("─┼─");
    }

    printf("\n" RESET);
}

void printTableRow(int columns, ...)
{
    va_list args;
    va_start(args, columns);

    printf("  ");

    for (int i = 0; i < columns; i++)
    {
        const char *value = va_arg(args, const char *);

        printf(WHITE "%-*.*s" RESET,
               TABLE_COL_WIDTH,
               TABLE_COL_WIDTH,
               value ? value : "");

        if (i < columns - 1)
            printf(" | ");
    }

    printf("\n");

    va_end(args);
}

void showInfo(const char *msg)
{
    printf("\n" CYAN "  [INFO] %s\n" RESET, msg);
}
