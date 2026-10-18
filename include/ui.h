// ui.h
#ifndef UI_H
#define UI_H

// Colors for Ubuntu Terminal
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"
#define DIM     "\x1b[2m"
#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"

// Internal UI functions
void clearScreen();
void showSplash();
void drawHeader(const char* title);
void drawMenuItem(int num, const char* title, const char* desc);
void drawFooter();

// Your Menu functions
void displayMainMenu(void);
void displayCustomerMenu(void);
void displayAdminMenu(void);

#endif