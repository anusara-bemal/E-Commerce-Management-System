#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/admin.h"
#include "../include/ui.h"

#define LOW_STOCK_THRESHOLD 50

struct ProductInfo {
    int id;
    char name[60];
    char category[20];
    float price;
    int stock;
};

// data/ files වලින් count ගන්න
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

// products.txt parse කරනවා - ඔයාගේ format එකට
static int loadProducts(struct ProductInfo products[], int max) {
    FILE *fp = fopen("data/products.txt", "r");
    if (!fp) return 0;
    char line[512];
    int count = 0;
    while (count < max && fgets(line, sizeof(line), fp)) {
        if (strlen(line) < 10) continue;
        // strtok වලින් | වලින් split කරනවා
        char *token;
        token = strtok(line, "|");
        if (!token) continue;
        products[count].id = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(products[count].name, token, 59);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(products[count].category, token, 19);

        token = strtok(NULL, "|");
        if (!token) continue;
        products[count].price = atof(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        products[count].stock = atoi(token);

        // description එක skip කරනවා
        count++;
    }
    fclose(fp);
    return count;
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

        // quick stats top එකේ
        printf(CYAN "  |  " RESET WHITE BOLD "Products: %d  Users: %d  Orders: %d\n" RESET, productCount, userCount, orderCount);
        printf(CYAN "  +----------------------------------------------------------+\n" RESET);

        drawMenuItem(1, "System Overview", "Counts & totals");
        drawMenuItem(2, "Product Statistics", "Stock analysis");
        drawMenuItem(3, "Sales Statistics", "Categories");
        drawMenuItem(4, "User Statistics", "From users.txt");
        drawMenuItem(5, "Revenue Report", "Inventory value");
        drawMenuItem(6, "Inventory Report", "Low / Out of stock");
        drawMenuItem(0, "Back", "Admin menu");

        drawFooter();

        if (scanf("%d", &choice) != 1) { int c; while((c=getchar())!='\n' && c!=EOF); choice=-1; continue; }
        int c; while((c=getchar())!='\n' && c!=EOF);

        if (choice == 0) break;

        // calculate common stats
        float totalValue = 0;
        int totalStock = 0;
        int lowStock = 0, outOfStock = 0;
        for(int i=0;i<productCount;i++){
            totalValue += products[i].price * products[i].stock;
            totalStock += products[i].stock;
            if(products[i].stock == 0) outOfStock++;
            else if(products[i].stock <= LOW_STOCK_THRESHOLD) lowStock++;
        }

        clearScreen();
        switch(choice){
            case 1:
                drawHeader("SYSTEM OVERVIEW");
                printf(GREEN "  Total Products      : %d\n" RESET, productCount);
                printf(GREEN "  Total Users         : %d\n" RESET, userCount);
                printf(GREEN "  Total Orders        : %d\n" RESET, orderCount);
                printf(GREEN "  Total Stock Units   : %d\n" RESET, totalStock);
                printf(GREEN "  Inventory Value     : Rs. %.2f\n" RESET, totalValue);
                printf(YELLOW "  Low Stock (<=%d)   : %d\n" RESET, LOW_STOCK_THRESHOLD, lowStock);
                printf(RED "  Out of Stock        : %d\n" RESET, outOfStock);
                break;
            case 2:
                drawHeader("PRODUCT STATISTICS");
                printf(WHITE BOLD "  %-6s %-30s %-10s %-6s\n" RESET, "ID", "NAME", "PRICE", "STOCK");
                printf(DIM "  ----------------------------------------------------------\n" RESET);
                for(int i=0;i<productCount && i<20;i++){
                    printf("  %-6d %-30.30s Rs.%-7.0f %-6d\n", products[i].id, products[i].name, products[i].price, products[i].stock);
                }
                if(productCount>20) printf(DIM "  ... and %d more products\n" RESET, productCount-20);
                break;
            case 3:
                drawHeader("SALES STATISTICS BY CATEGORY");
                {
                    // simple category count
                    char cats[20][20]; int catCount[20]={0}; int catTypes=0;
                    for(int i=0;i<productCount;i++){
                        int found=0;
                        for(int j=0;j<catTypes;j++) if(strcmp(cats[j], products[i].category)==0){catCount[j]++; found=1; break;}
                        if(!found && catTypes<20){ strcpy(cats[catTypes], products[i].category); catCount[catTypes]=1; catTypes++; }
                    }
                    for(int j=0;j<catTypes;j++){
                        printf("  %-15s : %d products\n", cats[j], catCount[j]);
                    }
                }
                printf("\n  Total Orders from file: %d\n", orderCount);
                break;
            case 4:
                drawHeader("USER STATISTICS");
                printf("  Total Users: %d (data/users.txt)\n", userCount);
                FILE *uf = fopen("data/users.txt","r");
                if(uf){ char l[300]; int i=1; while(fgets(l,sizeof(l),uf) && i<=15){ if(strlen(l)>3) printf("  %d. %s", i++, l); } fclose(uf); if(userCount>15) printf("  ... and %d more\n", userCount-15); }
                break;
            case 5:
                drawHeader("REVENUE REPORT");
                printf(GREEN BOLD "  Total Inventory Value : Rs. %.2f\n" RESET, totalValue);
                printf("  Total Stock Units     : %d\n", totalStock);
                printf("  Average Price         : Rs. %.2f\n", productCount?totalValue/productCount:0);
                // top expensive
                printf("\n  Top 5 Expensive Products:\n");
                for(int k=0;k<5 && k<productCount;k++){
                    int maxIdx=k; for(int i=k;i<productCount;i++) if(products[i].price>products[maxIdx].price) maxIdx=i;
                    struct ProductInfo tmp=products[k]; products[k]=products[maxIdx]; products[maxIdx]=tmp;
                    printf("    %d. %s - Rs. %.2f\n", k+1, products[k].name, products[k].price);
                }
                break;
            case 6:
                drawHeader("INVENTORY REPORT");
                printf(YELLOW "  Low Stock (<=%d units): %d products\n" RESET, LOW_STOCK_THRESHOLD, lowStock);
                printf(RED "  Out of Stock: %d products\n\n" RESET, outOfStock);
                printf(WHITE "  Low Stock List:\n" RESET);
                int shown=0;
                for(int i=0;i<productCount && shown<10;i++){
                    if(products[i].stock>0 && products[i].stock<=LOW_STOCK_THRESHOLD){
                        printf("    ID %d | %s | Stock: %d\n", products[i].id, products[i].name, products[i].stock);
                        shown++;
                    }
                }
                if(shown==0) showSuccess("No low stock items! All good.");
                break;
            default:
                showError("Invalid choice!");
                break;
        }
        pressEnterToContinue();
    } while(1);
}