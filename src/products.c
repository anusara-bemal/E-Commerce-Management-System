#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Product Structure Definition
typedef struct {
    int productId;
    char productName[50];
    char category[30];
    float price;
    int stockQuantity;
    char description[100];
} Product;

// Function Prototypes
void addProduct();
void viewProducts();
void searchProduct();
void updateProduct();
void deleteProduct();
void productMenu();

// Helper Functions for Stock Management
int checkStockAvailability(int productId, int requiredQty);
void updateStockAfterPurchase(int productId, int purchasedQty);

int main() {
    productMenu();
    return 0;
}

// Product Management Menu
void productMenu() {
    int choice;
    do {
        printf("\n========================================\n");
        printf("       ADMIN PRODUCT MANAGEMENT         \n");
        printf("========================================\n");
        printf("1. Add Product\n");
        printf("2. View Products\n");
        printf("3. Search Product\n");
        printf("4. Update Product\n");
        printf("5. Delete Product\n");
        printf("6. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar(); // Clear input buffer

        switch (choice) {
            case 1:
                addProduct();
                break;
            case 2:
                viewProducts();
                break;
            case 3:
                searchProduct();
                break;
            case 4:
                updateProduct();
                break;
            case 5:
                deleteProduct();
                break;
            case 6:
                printf("Returning to Main Menu...\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 6);
}

// 1. Add Product Function
void addProduct() {
    FILE *file = fopen("products.dat", "ab");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    Product p;
    printf("\n--- Add New Product ---\n");
    printf("Enter Product ID: ");
    scanf("%d", &p.productId);
    getchar(); // Clear buffer

    printf("Enter Product Name: ");
    fgets(p.productName, sizeof(p.productName), stdin);
    p.productName[strcspn(p.productName, "\n")] = 0; // Remove newline

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

    printf("Product added successfully!\n");
}

// 2. View All Products Function
void viewProducts() {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) {
        printf("No products found! (File does not exist yet)\n");
        return;
    }

    Product p;
    printf("\n---------------------------------------------------------------------------------\n");
    printf("%-10s %-20s %-15s %-12s %-15s %-20s\n", "Product ID", "Product Name", "Category", "Price", "Stock Qty", "Description");
    printf("---------------------------------------------------------------------------------\n");

    while (fread(&p, sizeof(Product), 1, file)) {
        printf("%-10d %-20s %-15s %-12.2f %-15d %-20s\n", 
               p.productId, p.productName, p.category, p.price, p.stockQuantity, p.description);
    }
    printf("---------------------------------------------------------------------------------\n");
    fclose(file);
}

// 3. Search Product Function
void searchProduct() {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) {
        printf("No products available to search.\n");
        return;
    }

    int searchId, found = 0;
    printf("\nEnter Product ID to search: ");
    scanf("%d", &searchId);

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) {
            printf("\nProduct Found!\n");
            printf("Product ID    : %d\n", p.productId);
            printf("Product Name  : %s\n", p.productName);
            printf("Category      : %s\n", p.category);
            printf("Price         : %.2f\n", p.price);
            printf("Stock Quantity: %d\n", p.stockQuantity);
            printf("Description   : %s\n", p.description);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("Product with ID %d not found.\n", searchId);
    }
    fclose(file);
}

// 4. Update Product Function
void updateProduct() {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) {
        printf("No products available to update.\n");
        return;
    }

    FILE *tempFile = fopen("temp.dat", "wb");
    if (tempFile == NULL) {
        printf("Error opening temporary file!\n");
        fclose(file);
        return;
    }

    int searchId, found = 0;
    printf("\nEnter Product ID to update: ");
    scanf("%d", &searchId);
    getchar();

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) {
            found = 1;
            printf("Product found! Enter new details:\n");
            
            printf("Enter New Product Name [%s]: ", p.productName);
            char input[50];
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) strcpy(p.productName, input);

            printf("Enter New Category [%s]: ", p.category);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) strcpy(p.category, input);

            printf("Enter New Price [%.2f]: ", p.price);
            char priceStr[20];
            fgets(priceStr, sizeof(priceStr), stdin);
            if (strlen(priceStr) > 1) p.price = atof(priceStr);

            printf("Enter New Stock Quantity [%d]: ", p.stockQuantity);
            char stockStr[20];
            fgets(stockStr, sizeof(stockStr), stdin);
            if (strlen(stockStr) > 1) p.stockQuantity = atoi(stockStr);

            printf("Enter New Description [%s]: ", p.description);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) strcpy(p.description, input);
        }
        fwrite(&p, sizeof(Product), 1, tempFile);
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove("products.dat");
        rename("temp.dat", "products.dat");
        printf("Product updated successfully!\n");
    } else {
        remove("temp.dat");
        printf("Product with ID %d not found.\n", searchId);
    }
}

// 5. Delete Product Function
void deleteProduct() {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) {
        printf("No products available to delete.\n");
        return;
    }

    FILE *tempFile = fopen("temp.dat", "wb");
    if (tempFile == NULL) {
        printf("Error opening temporary file!\n");
        fclose(file);
        return;
    }

    int searchId, found = 0;
    printf("\nEnter Product ID to delete: ");
    scanf("%d", &searchId);

    Product p;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == searchId) {
            found = 1;
            continue; 
        }
        fwrite(&p, sizeof(Product), 1, tempFile);
    }

    fclose(file);
    fclose(tempFile);

    if (found) {
        remove("products.dat");
        rename("temp.dat", "products.dat");
        printf("Product deleted successfully!\n");
    } else {
        remove("temp.dat");
        printf("Product with ID %d not found.\n", searchId);
    }
}

// --- Stock Management Helper Functions ---

int checkStockAvailability(int productId, int requiredQty) {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) return 0;

    Product p;
    int available = 0;
    while (fread(&p, sizeof(Product), 1, file)) {
        if (p.productId == productId) {
            if (p.stockQuantity >= requiredQty) {
                available = 1;
            }
            break;
        }
    }
    fclose(file);
    return available;
}

void updateStockAfterPurchase(int productId, int purchasedQty) {
    FILE *file = fopen("products.dat", "rb");
    if (file == NULL) return;

    FILE *tempFile = fopen("temp.dat", "wb");
    if (tempFile == NULL) {
        fclose(file);
        return;
    }

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
    remove("products.dat");
    rename("temp.dat", "products.dat");
}