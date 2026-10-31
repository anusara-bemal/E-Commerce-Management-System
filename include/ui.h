#ifndef UI_H
#define UI_H

#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"
#define DIM     "\x1b[2m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"

// THEME - direct colors, no chaining
#define THEME_PRIMARY MAGENTA
#define THEME_ACCENT CYAN

#define R_TL "╭"
#define R_TR "╮"
#define R_BL "╰"
#define R_BR "╯"
#define R_H  "─"
#define R_V  "│"

void clearScreen(void);
void showSplash(void);
void drawHeader(const char* title);
void drawFooter(void);
void drawMenuItem(int num, const char* text, const char* desc);
void displayMainMenu(void);
void displayAdminMenu(void);
void displayCustomerMenu(const char* username);
void showSuccess(const char* msg);
void showError(const char* msg);
void showWarning(const char* msg);
void pressEnterToContinue(void);
void printTableHeader(int columns, ...);
void printTableDivider(int columns);
void printTableRow(int columns, ...);
void showInfo(const char *message);

#endif