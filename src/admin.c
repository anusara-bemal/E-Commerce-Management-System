#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/admin.h"
#include "../include/ui.h"

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_EMAIL 100
#define MAX_tpNumber 20
#define MAX_Adress 100
#define MAX_USERS 200
#define LOW_STOCK_THRESHOLD 50

typedef struct {
    int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char email[MAX_EMAIL];
    char tpNumber[MAX_tpNumber];
    char address[MAX_Adress];
    int role; // 1 = admin, 0 = user
    int isActive;
} User;

struct ProductInfo {
    int id;
    char name[60];
    char category[20];
    float price;
    int stock;
};

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

static int loadProducts(struct ProductInfo products[], int max) {
    FILE *fp = fopen("data/products.txt", "r");
    if (!fp) return 0;
    char line[512]; 
    int count = 0;
    while (count < max && fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 10) continue;
        char *t = strtok(line, "|");
        if (!t) continue;
        products[count].id = atoi(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(products[count].name, t, 59);
        products[count].name[59] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(products[count].category, t, 19);
        products[count].category[19] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        products[count].price = atof(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        products[count].stock = atoi(t);
        count++;
    }
    fclose(fp); 
    return count;
}

// ================= USER MANAGEMENT =================
static int loadUsers(User users[], int max) {
    FILE *fp = fopen("data/users.txt", "r");
    if (!fp) return 0;
    char line[600]; 
    int count = 0;
    while (count < max && fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 10) continue;
        User u;
        char *t = strtok(line, "|");
        if (!t) continue;
        u.id = atoi(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(u.username, t, MAX_USERNAME-1);
        u.username[MAX_USERNAME-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(u.password, t, MAX_PASSWORD-1);
        u.password[MAX_PASSWORD-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(u.email, t, MAX_EMAIL-1);
        u.email[MAX_EMAIL-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(u.tpNumber, t, MAX_tpNumber-1);
        u.tpNumber[MAX_tpNumber-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        strncpy(u.address, t, MAX_Adress-1);
        u.address[MAX_Adress-1] = '\0';

        t = strtok(NULL, "|");
        if (!t) continue;
        u.role = atoi(t);

        t = strtok(NULL, "|");
        if (!t) continue;
        u.isActive = atoi(t);

        users[count++] = u;
    }
    fclose(fp); 
    return count;
}

static int saveUsers(User users[], int count) {
    FILE *fp = fopen("data/users.txt", "w");
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
        int userCount = loadUsers(users, MAX_USERS);
        clearScreen();
        drawHeader("USER MANAGEMENT");
        printf(CYAN "  |  " RESET WHITE BOLD "Total Users: %d\n" RESET, userCount);
        printf(CYAN "  +----------------------------------------------------------+\n" RESET);
        printf(WHITE BOLD "  %-4s %-15s %-25s %-6s\n" RESET, "ID", "USERNAME", "EMAIL", "ROLE");
        printf(DIM "  ----------------------------------------------------------\n" RESET);
        for (int i = 0; i < userCount; i++) {
            const char* roleStr = (users[i].role == 1) ? "ADMIN" : "USER";
            printf("  %-4d %-15.15s %-25.25s %-6s\n",
                users[i].id, users[i].username, users[i].email, roleStr);
        }
        printf(CYAN "\n  +----------------------------------------------------------+\n" RESET);
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
            case 1:
                printf(YELLOW "\n  Are you sure to delete user %s? (y/n): " RESET, users[found].username);
                char confirm = getchar(); while((c=getchar())!='\n' && c!=EOF);
                if (confirm=='y' || confirm=='Y') {
                    for (int i = found; i < userCount-1; i++) users[i] = users[i+1];
                    userCount--;
                    if (saveUsers(users, userCount)) showSuccess("User deleted successfully!");
                    else showError("Failed to save!");
                } else { showWarning("Cancelled"); }
                break;
            case 2:
                users[found].role = 1;
                if (saveUsers(users, userCount)) {
                    char msg[100]; sprintf(msg, "%s is now ADMIN", users[found].username);
                    showSuccess(msg);
                } else showError("Save failed");
                break;
            case 3:
                users[found].role = 0;
                if (saveUsers(users, userCount)) {
                    char msg[100]; sprintf(msg, "%s is now USER", users[found].username);
                    showSuccess(msg);
                } else showError("Save failed");
                break;
            default: showError("Invalid choice!"); break;
        }
        pressEnterToContinue();
    } while(1);
}

void showReports(void) {
    struct ProductInfo products[200];
    int productCount = loadProducts(products, 200);
    int userCount = getLineCount("data/users.txt");
    int orderCount = getLineCount("data/orders.txt");
    int choice;
    do {
        clearScreen();
        drawHeader("E-COMMERCE DASHBOARD");
        printf(CYAN "  |  " RESET WHITE BOLD "Products: %d  Users: %d  Orders: %d\n" RESET, productCount, userCount, orderCount);
        printf(CYAN "  +----------------------------------------------------------+\n" RESET);
        drawMenuItem(1, "System Overview", "Counts & totals");
        drawMenuItem(2, "Product Statistics", "Stock analysis");
        drawMenuItem(3, "Inventory Report", "Low stock");
        drawMenuItem(0, "Back", "Admin menu");
        drawFooter();
        if (scanf("%d", &choice) != 1) { int c; while((c=getchar())!='\n' && c!=EOF); choice=-1; continue; }
        int c; while((c=getchar())!='\n' && c!=EOF);
        if (choice == 0) break;
        float totalValue=0; int totalStock=0, lowStock=0, out=0;
        for(int i=0;i<productCount;i++){ totalValue+=products[i].price*products[i].stock; totalStock+=products[i].stock; if(products[i].stock==0) out++; else if(products[i].stock<=LOW_STOCK_THRESHOLD) lowStock++; }
        clearScreen();
        if(choice==1){ drawHeader("SYSTEM OVERVIEW"); printf(GREEN "  Products: %d\n  Users: %d\n  Orders: %d\n  Stock: %d\n  Value: Rs. %.2f\n" RESET, productCount, userCount, orderCount, totalStock, totalValue); }
        else if(choice==2){ drawHeader("PRODUCT STATS"); for(int i=0;i<productCount && i<20;i++) printf("  %-4d %-30.30s Rs.%.0f (%d)\n", products[i].id, products[i].name, products[i].price, products[i].stock); }
        else if(choice==3){ drawHeader("INVENTORY"); printf(YELLOW "  Low: %d Out: %d\n" RESET, lowStock, out); }
        pressEnterToContinue();
    } while(1);
}