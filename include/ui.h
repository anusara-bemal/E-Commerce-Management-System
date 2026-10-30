#ifndef UI_H
#define UI_H

#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"
#define DIM     "\x1b[2m"

#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define RED     "\x1b[31m"
#define BLUE    "\x1b[34m"
#define WHITE   "\x1b[97m"
#define MAGENTA "\x1b[35m"

#define UI_WIDTH 60

void clearScreen(void);
void showSplash(void);

void drawHeader(const char *title);
void drawMenuItem(int num, const char *title, const char *desc);
void drawFooter(void);

void showSuccess(const char *message);
void showError(const char *message);
void showWarning(const char *message);
void pressEnterToContinue(void);

void displayMainMenu(void);
void displayCustomerMenu(void);
void displayAdminMenu(void);

#endif
