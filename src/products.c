#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "products.h"
#include "ui.h"

// Forward declarations - error එක නැති කරන්න
void productMenu(void);
void addProduct(void);
void viewProducts(void);
void searchProduct(void);
void updateProduct(void);
void deleteProduct(void);
int checkStockAvailability(int productId, int requiredQty);
void updateStockAfterPurchase(int productId, int purchasedQty);

// Product Management Menu
void productMenu(void) {
    int choice;
    do {
        clearScreen();
        drawHeader("ADMIN PRODUCT MANAGEMENT");
        printf("1. Add Product\n");
        printf("2. View Products\n");
        printf("3. Search Product\n");
        printf("4. Update Product\n");
        printf("5. Delete Product\n");
        printf("6. Back\n");
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }
        getchar();

        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: searchProduct(); break;
            case 4: updateProduct(); break;
            case 5: deleteProduct(); break;
            case 6: printf("Returning...\n"); break;
            default: printf("Invalid choice!\n");
        }
        if (choice != 6) {
            printf("\nPress Enter...");
            getchar();
        }
    } while (choice != 6);
}

void addProduct(void) {
    FILE *file = fopen("data/products.txt", "a");
    if (file == NULL) {
        file = fopen("products.txt", "a");
        if (file == NULL) {
            printf("Error opening file!\n");
            return;
        }
    }

    Product p;
    printf("\n--- Add New Product ---\n");
    printf("Enter Product ID: ");
    scanf("%d", &p.productId);
    getchar();

    printf("Enter Product Name: ");
    fgets(p.productName, sizeof(p.productName), stdin);
    p.productName[strcspn(p.productName, "\n")] = 0;

    printf("Enter Category: ");
    fgets(p.category, sizeof(p.category), stdin);
    p.category[strcspn(p.category, "\n")] = 0;

    printf("Enter Price: ");
    scanf("%f", &p.price);

    printf("Enter Stock Quantity: ");
    scanf("%d", &p.stockQuantity);
    getchar();

    printf("Enter Description: ");
    fgets(p.description, sizeof(p.description), stdin);
    p.description[strcspn(p.description, "\n")] = 0;

    fwrite(&p, sizeof(Product), 1, file);
    fclose(file);
    printf("\n%sProduct added successfully!%s\n", GREEN, RESET);
}

void viewProducts(void) {
    FILE *file = fopen("data/products.txt", "rb");
    if (file == NULL) file = fopen("products.txt", "r");
    if (file == NULL) {
        printf("No products found! Add products first.\n");
        return;
    }

    Product p;
    printf("\n---------------------------------------------------------------------------------\n");
    printf("%-10s %-20s %-15s %-10s %-8s\n", "ID", "Name", "Category", "Price", "Stock");
    printf("---------------------------------------------------------------------------------\n");
    int count = 0;
    while (fread(&p, sizeof(Product), 1, file)) {
        printf("%-10d %-20s %-15s %-10.2f %-8d\n", 
               p.productId, p.productName, p.category, p.price, p.stockQuantity);
        count++;
    }
    if (count == 0) printf("No products yet.\n");
    printf("---------------------------------------------------------------------------------\n");
    fclose(file);
}

void searchProduct(void) {
    FILE *file = fopen("data/products.txt", "r");
    if (file == NULL) file = fopen("products.txt", "r");
    if (file == NULL) {
        printf("No products available.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter Product ID to search: ");
    scanf("%d", &searchId);

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) {
            printf("\n--- Product Found! ---\n");
            printf("ID: %d\nName: %s\nCategory: %s\nPrice: %.2f\nStock: %d\nDesc: %s\n", 
                p.productId, p.productName, p.category, p.price, p.stockQuantity, p.description);
            found = 1;
            break;
        }
    }
    if (!found) printf("Product ID %d not found.\n", searchId);
    fclose(file);
}

void updateProduct(void) {
    FILE *file = fopen("data/products.txt", "r");
    if (file == NULL) file = fopen("products.txt", "r");
    if (file == NULL) { printf("No products to update.\n"); return; }

    FILE *tempFile = fopen("data/temp.txt", "w");
    if (tempFile == NULL) tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL) { fclose(file); return; }

    int searchId, found = 0;
    printf("\nEnter Product ID to update: ");
    scanf("%d", &searchId);
    getchar();

    Product p;
    char input[100];
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) {
            found = 1;
            printf("Found! Enter new details (leave empty to keep old):\n");
            printf("Name [%s]: ", p.productName);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) strcpy(p.productName, input);

            printf("Category [%s]: ", p.category);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) strcpy(p.category, input);

            printf("Price [%.2f]: ", p.price);
            fgets(input, sizeof(input), stdin);
            if (strlen(input) > 1) p.price = atof(input);

            printf("Stock [%d]: ", p.stockQuantity);
            fgets(input, sizeof(input), stdin);
            if (strlen(input) > 1) p.stockQuantity = atoi(input);
        }
        fwrite(&p, sizeof(Product), 1, tempFile);
    }
    fclose(file);
    fclose(tempFile);

    if (found) {
        remove("data/products.txt");
        remove("products.txt");
        rename("txta/temp.txt", "data/products.txt");
        rename("temp.txt", "products.txt");
        printf("Product updated!\n");
    } else {
        remove("data/temp.txt");
        remove("temp.txt");
        printf("Not found.\n");
    }
}

void deleteProduct(void) {
    FILE *file = fopen("data/products.txt", "rb");
    if (file == NULL) file = fopen("products.txt", "rb");
    if (file == NULL) { printf("No products to delete.\n"); return; }

    FILE *tempFile = fopen("data/temp.txt", "wb");
    if (tempFile == NULL) tempFile = fopen("temp.txt", "wb");

    int searchId, found = 0;
    printf("\nEnter Product ID to delete: ");
    scanf("%d", &searchId);

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) { found = 1; continue; }
        fwrite(&p, sizeof(Product), 1, tempFile);
    }
    fclose(file);
    fclose(tempFile);

    if (found) {
        remove("data/products.txt");
        remove("products.txt");
        rename("data/temp.txt", "data/products.txt");
        rename("temp.txt", "products.txt");
        printf("Product deleted!\n");
    } else {
        remove("data/temp.txt");
        remove("temp.txt");
        printf("ID %d not found.\n", searchId);
    }
}

int checkStockAvailability(int productId, int requiredQty) {
    FILE *file = fopen("data/products.txt", "rb");
    if (file == NULL) file = fopen("products.txt", "rb");
    if (file == NULL) return 0;
    Product p;
    int available = 0;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == productId && p.stockQuantity >= requiredQty) {
            available = 1; break;
        }
    }
    fclose(file);
    return available;
}

void updateStockAfterPurchase(int productId, int purchasedQty) {
    FILE *file = fopen("data/products.txt", "rb");
    if (file == NULL) file = fopen("products.txt", "rb");
    if (file == NULL) return;
    FILE *tempFile = fopen("data/temp.txt", "wb");
    if (tempFile == NULL) tempFile = fopen("temp.txt", "wb");
    if (tempFile == NULL) { fclose(file); return; }

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == productId) {
            p.stockQuantity -= purchasedQty;
            if (p.stockQuantity < 0) p.stockQuantity = 0;
        }
        fwrite(&p, sizeof(Product), 1, tempFile);
    }
    fclose(file);
    fclose(tempFile);
    remove("data/products.txt");
    remove("products.txt");
    rename("data/temp.txt", "data/products.txt");
    rename("temp.txt", "products.txt");
}