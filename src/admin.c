#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "users.h"
#include "products.h"
#include "admin.h"
#include "ui.h"

#ifndef MAX_USERS
#define MAX_USERS 200
#endif
#ifndef LOW_STOCK_THRESHOLD
#define LOW_STOCK_THRESHOLD 10
#endif

// ================= COMMON HELPERS =================
static int getLineCount(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;
    int count = 0; 
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strlen(line) > 5) count++;
    }
    fclose(fp);
    return count;
}

static int loadProductsLocal(Product products[], int max) {
    FILE *fp = fopen(PRODUCTS_FILE, "r");
    if (!fp) return 0;
    char line[512]; 
    int count = 0;
    while (count < max && fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 10) continue;
        char *t = strtok(line, "|");
        if (!t) continue;
        products[count].productId = atoi(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(products[count].productName, t, MAX_PRODUCT_NAME-1);
        products[count].productName[MAX_PRODUCT_NAME-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(products[count].category, t, MAX_CATEGORY-1);
        products[count].category[MAX_CATEGORY-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        products[count].price = atof(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        products[count].stockQuantity = atoi(t);

        // description optional
        t = strtok(NULL, "|");
        if (t) {
            strncpy(products[count].description, t, DESCRIPTION-1);
            products[count].description[DESCRIPTION-1] = '\0';
        }
        count++;
    }
    fclose(fp); 
    return count;
}

static int loadUsersLocal(User users[], int max) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;
    char line[600]; 
    int count = 0;
    while (count < max && fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 10) continue;
        User u;
        memset(&u, 0, sizeof(User));
        char *t = strtok(line, "|");
        if (!t) continue;
        u.id = atoi(t);
        t = strtok(NULL, "|"); if (!t) continue; strncpy(u.username, t, MAX_USERNAME-1);
        t = strtok(NULL, "|"); if (!t) continue; strncpy(u.password, t, MAX_PASSWORD-1);
        t = strtok(NULL, "|"); if (!t) continue; strncpy(u.email, t, MAX_EMAIL-1);
        t = strtok(NULL, "|"); if (!t) continue; strncpy(u.tpNumber, t, MAX_tpNumber-1);
        t = strtok(NULL, "|"); if (!t) continue; strncpy(u.address, t, MAX_Adress-1);
        t = strtok(NULL, "|"); if (!t) continue; u.role = atoi(t);
        t = strtok(NULL, "|"); if (!t) continue; u.isActive = atoi(t);
        users[count++] = u;
    }
    fclose(fp); 
    return count;
}

static int saveUsersLocal(User users[], int count) {
    FILE *fp = fopen(USERS_FILE, "w");
    if (!fp) return 0;
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%d|%s|%s|%s|%s|%s|%d|%d\n",
            users[i].id, users[i].username, users[i].password,
            users[i].email, users[i].tpNumber, users[i].address,
            users[i].role, users[i].isActive);
    }
    fclose(fp); 
    return 1;
}

void manageUsers(void) {
    User users[MAX_USERS];
    int choice;
    do {
        int userCount = loadUsersLocal(users, MAX_USERS);
        clearScreen();
        drawHeader("USER MANAGEMENT");
        printf(CYAN "  │  Total Users: %d\n" RESET, userCount);
        printf(CYAN "  ├──────────────────────────────────────────────────────────┤\n" RESET);
        printf(WHITE BOLD "  %-4s %-15s %-25s %-6s\n" RESET, "ID", "USERNAME", "EMAIL", "ROLE");
        printf(DIM "  ──────────────────────────────────────────────────────────\n" RESET);
        for (int i = 0; i < userCount; i++) {
            const char* roleStr = (users[i].role == ROLE_ADMIN) ? "ADMIN" : "USER";
            printf("  %-4d %-15.15s %-25.25s %-6s\n", users[i].id, users[i].username, users[i].email, roleStr);
        }
        printf(CYAN "\n  ├──────────────────────────────────────────────────────────┤\n" RESET);
        drawMenuItem(1, "Delete User", "By User ID");
        drawMenuItem(2, "Make as Admin", "Set role=1");
        drawMenuItem(3, "Make as User", "Set role=0");
        drawMenuItem(0, "Back", "Admin menu");
        drawFooter();
        if (scanf("%d", &choice) != 1) { int c; while((c=getchar())!='\n' && c!=EOF); choice=-1; continue; }
        int c; while((c=getchar())!='\n' && c!=EOF);
        if (choice == 0) break;
        int targetId;
        printf("\n  Enter User ID: ");
        if (scanf("%d", &targetId) != 1) { showError("Invalid ID"); pressEnterToContinue(); continue; }
        while((c=getchar())!='\n' && c!=EOF);
        int found = -1;
        for (int i = 0; i < userCount; i++) if (users[i].id == targetId) { found = i; break; }
        if (found == -1) { showError("User ID not found!"); pressEnterToContinue(); continue; }
        switch(choice) {
            case 1: {
                printf(YELLOW "\n  Are you sure to delete user %s? (y/n): " RESET, users[found].username);
                char confirm = getchar(); while((c=getchar())!='\n' && c!=EOF);
                if (confirm=='y' || confirm=='Y') {
                    for (int i = found; i < userCount-1; i++) users[i] = users[i+1];
                    userCount--;
                    if (saveUsersLocal(users, userCount)) showSuccess("User deleted successfully!");
                    else showError("Failed to save!");
                } else { showWarning("Cancelled"); }
                break;
            }
            case 2:
                users[found].role = ROLE_ADMIN;
                if (saveUsersLocal(users, userCount)) { char msg[100]; sprintf(msg, "%s is now ADMIN", users[found].username); showSuccess(msg); }
                else showError("Save failed");
                break;
            case 3:
                users[found].role = ROLE_CUSTOMER;
                if (saveUsersLocal(users, userCount)) { char msg[100]; sprintf(msg, "%s is now USER", users[found].username); showSuccess(msg); }
                else showError("Save failed");
                break;
            default: showError("Invalid choice!"); break;
        }
        pressEnterToContinue();
    } while(1);
}

void showReports(void) {
    Product products[200];
    int productCount = loadProductsLocal(products, 200);
    int userCount = getLineCount(USERS_FILE);
    int orderCount = getLineCount(ORDERS_FILE);
    int choice;
    do {
        clearScreen();
        drawHeader("E-COMMERCE DASHBOARD");
        printf(CYAN "  │  Products: %d  Users: %d  Orders: %d\n" RESET, productCount, userCount, orderCount);
        printf(CYAN "  ├──────────────────────────────────────────────────────────┤\n" RESET);
        drawMenuItem(1, "System Overview", "Counts & totals");
        drawMenuItem(2, "Product Statistics", "Stock analysis");
        drawMenuItem(3, "Inventory Report", "Low stock");
        drawMenuItem(0, "Back", "Admin menu");
        drawFooter();
        if (scanf("%d", &choice) != 1) { int c; while((c=getchar())!='\n' && c!=EOF); choice=-1; continue; }
        int c; while((c=getchar())!='\n' && c!=EOF);
        if (choice == 0) break;
        float totalValue=0; int totalStock=0, lowStock=0, out=0;
        for(int i=0;i<productCount;i++){ totalValue+=products[i].price*products[i].stockQuantity; totalStock+=products[i].stockQuantity; if(products[i].stockQuantity==0) out++; else if(products[i].stockQuantity<=LOW_STOCK_THRESHOLD) lowStock++; }
        clearScreen();
        if(choice==1){ drawHeader("SYSTEM OVERVIEW"); printf(GREEN "  Products: %d\n  Users: %d\n  Orders: %d\n  Stock: %d\n  Value: Rs. %.2f\n" RESET, productCount, userCount, orderCount, totalStock, totalValue); }
        else if(choice==2){ drawHeader("PRODUCT STATS"); for(int i=0;i<productCount && i<20;i++) printf("  %-4d %-30.30s Rs.%.0f (%d)\n", products[i].productId, products[i].productName, products[i].price, products[i].stockQuantity); }
        else if(choice==3){ drawHeader("INVENTORY"); printf(YELLOW "  Low: %d Out: %d\n" RESET, lowStock, out); }
        pressEnterToContinue();
    } while(1);
}