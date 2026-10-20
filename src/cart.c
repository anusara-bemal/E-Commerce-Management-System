#include "../include/cart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =================================================================
// STRUCT DEFINITIONS
// =================================================================
typedef struct {
    int productId;
    char productName[50];
    char category[30];
    float price;
    int stockQuantity;
    char description[100];
} Product;

typedef struct {
    char customerName[50];
    int productId;
    char productName[50];
    int quantity;
    float unitPrice;
    float subtotal;
} CartItem;

// Internal Function Prototype
void updateCartQuantity(char* username);

// =================================================================
// 1. ADD TO CART FUNCTION
// =================================================================
void addToCart(char* username, int productId, int quantity) {
    Product product;
    CartItem cartItem;
    int found = 0;

    FILE *fpProd = fopen("products.dat", "rb");
    if (fpProd == NULL) {
        printf("[Error] Could not open products file!\n");
        return;
    }

    while (fread(&product, sizeof(Product), 1, fpProd)) {
        if (product.productId == productId) {
            found = 1;
            break;
        }
    }
    fclose(fpProd);

    if (!found) {
        printf("[Error] Product ID %d not found!\n", productId);
        return;
    }

    if (product.stockQuantity <= 0) {
        printf("[Error] Sorry, this product is out of stock!\n");
        return;
    }

    if (quantity <= 0 || quantity > product.stockQuantity) {
        printf("[Error] Invalid quantity or exceeds available stock!\n");
        return;
    }

    strcpy(cartItem.customerName, username);
    cartItem.productId = product.productId;
    strcpy(cartItem.productName, product.productName);
    cartItem.quantity = quantity;
    cartItem.unitPrice = product.price;
    cartItem.subtotal = quantity * product.price;

    FILE *fpCart = fopen("cart.dat", "ab");
    if (fpCart == NULL) {
        printf("[Error] Could not open cart file for writing!\n");
        return;
    }

    fwrite(&cartItem, sizeof(CartItem), 1, fpCart);
    fclose(fpCart);

    printf("\n[Success] %s (Qty: %d) added to your cart successfully!\n", cartItem.productName, cartItem.quantity);
}

// =================================================================
// 2. VIEW CART FUNCTION
// =================================================================
void viewCart(char* username) {
    CartItem cartItem;
    float totalAmount = 0.0;
    int choice = 5;

    do {
        system("cls"); // Windows නිසා "cls" ලෙස වෙනස් කරන ලදි
        printf("\n===============================================================\n");
        printf("                          SHOPPING CART                        \n");
        printf("===============================================================\n");
        printf("%-20s %-8s %-12s %-12s\n", "Product", "Qty", "Price", "Subtotal");
        printf("---------------------------------------------------------------\n");

        FILE *fpCart = fopen("cart.dat", "rb");
        int hasItems = 0;
        totalAmount = 0.0;

        if (fpCart != NULL) {
            while (fread(&cartItem, sizeof(CartItem), 1, fpCart)) {
                if (strcmp(cartItem.customerName, username) == 0) {
                    hasItems = 1;
                    printf("%-20s %-8d Rs. %-8.2f Rs. %-8.2f\n", 
                           cartItem.productName, cartItem.quantity, 
                           cartItem.unitPrice, cartItem.subtotal);
                    totalAmount += cartItem.subtotal;
                }
            }
            fclose(fpCart);
        }

        if (!hasItems) {
            printf("Your cart is currently empty.\n");
            printf("===============================================================\n");
            printf("\nPress Enter to return...");
            while (getchar() != '\n'); 
            getchar();
            return;
        } else {
            printf("---------------------------------------------------------------\n");
            printf("%-42s Rs. %.2f\n", "Total Amount:", totalAmount);
            printf("===============================================================\n");
        }

        printf("\nCart Options:\n");
        printf("1. Update item quantity\n");
        printf("2. Remove an item from cart\n");
        printf("3. Clear entire cart\n");
        printf("4. Proceed to Checkout\n");
        printf("5. Return to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        int targetId;

        switch (choice) {
            case 1:
                updateCartQuantity(username);
                break;
            case 2:
                printf("\nEnter Product ID to remove from cart: ");
                scanf("%d", &targetId);
                removeFromCart(username, targetId);
                break;
            case 3:
                clearCart(username);
                break;
            case 4:
                printf("\n[Info] Proceeding to checkout...\n");
                printf("Press Enter to continue...");
                while (getchar() != '\n');
                getchar();
                break;
            case 5:
                return;
            default:
                printf("[Error] Invalid choice! Press Enter to try again...");
                while (getchar() != '\n');
                getchar();
        }
    } while (choice != 5);
}

// =================================================================
// 3. UPDATE QUANTITY FUNCTION
// =================================================================
void updateCartQuantity(char* username) {
    int productId;
    int quantity;
    int found = 0;

    printf("\nEnter Product ID to update quantity: ");
    scanf("%d", &productId);

    Product product;
    int productFound = 0;
    FILE *fpProd = fopen("products.dat", "rb");
    if (fpProd != NULL) {
        while (fread(&product, sizeof(Product), 1, fpProd)) {
            if (product.productId == productId) {
                productFound = 1;
                break;
            }
        }
        fclose(fpProd);
    }

    if (!productFound) {
        printf("[Error] Product ID not found in inventory!\n");
        printf("Press Enter to continue...");
        while (getchar() != '\n');
        getchar();
        return;
    }

    printf("Enter new quantity (Available Stock: %d): ", product.stockQuantity);
    scanf("%d", &quantity);

    if (quantity <= 0 || quantity > product.stockQuantity) {
        printf("[Error] Invalid quantity or exceeds available stock!\n");
        printf("Press Enter to continue...");
        while (getchar() != '\n');
        getchar();
        return;
    }

    FILE *fpCart = fopen("cart.dat", "rb");
    if (fpCart == NULL) {
        printf("[Error] Could not open cart file!\n");
        return;
    }

    FILE *fpTemp = fopen("temp.dat", "wb");
    if (fpTemp == NULL) {
        printf("[Error] Could not create temporary file!\n");
        fclose(fpCart);
        return;
    }

    CartItem cartItem;
    while (fread(&cartItem, sizeof(CartItem), 1, fpCart)) {
        if (strcmp(cartItem.customerName, username) == 0 && cartItem.productId == productId) {
            cartItem.quantity = quantity;
            cartItem.subtotal = quantity * cartItem.unitPrice;
            found = 1;
        }
        fwrite(&cartItem, sizeof(CartItem), 1, fpTemp);
    }

    fclose(fpCart);
    fclose(fpTemp);

    remove("cart.dat");
    rename("temp.dat", "cart.dat");

    if (found) {
        printf("[Success] Cart quantity updated successfully!\n");
    } else {
        printf("[Error] Product ID %d not found in your cart!\n", productId);
    }
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    getchar();
}

// =================================================================
// 4. REMOVE ITEM FUNCTION
// =================================================================
void removeFromCart(char* username, int productId) {
    int found = 0;

    FILE *fpCart = fopen("cart.dat", "rb");
    if (fpCart == NULL) {
        printf("[Error] Could not open cart file!\n");
        return;
    }

    FILE *fpTemp = fopen("temp.dat", "wb");
    if (fpTemp == NULL) {
        printf("[Error] Could not create temporary file!\n");
        fclose(fpCart);
        return;
    }

    CartItem cartItem;
    while (fread(&cartItem, sizeof(CartItem), 1, fpCart)) {
        if (strcmp(cartItem.customerName, username) == 0 && cartItem.productId == productId) {
            found = 1;
        } else {
            fwrite(&cartItem, sizeof(CartItem), 1, fpTemp);
        }
    }

    fclose(fpCart);
    fclose(fpTemp);

    remove("cart.dat");
    rename("temp.dat", "cart.dat");

    if (found) {
        printf("[Success] Product ID %d removed from your cart.\n", productId);
    } else {
        printf("[Error] Product ID %d not found in your cart!\n", productId);
    }
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    getchar();
}

// =================================================================
// 5. CLEAR CART FUNCTION
// =================================================================
void clearCart(char* username) {
    FILE *fpCart = fopen("cart.dat", "rb");
    if (fpCart == NULL) {
        printf("[Error] Could not open cart file!\n");
        return;
    }

    FILE *fpTemp = fopen("temp.dat", "wb");
    if (fpTemp == NULL) {
        printf("[Error] Could not create temporary file!\n");
        fclose(fpCart);
        return;
    }

    CartItem cartItem;
    while (fread(&cartItem, sizeof(CartItem), 1, fpCart)) {
        if (strcmp(cartItem.customerName, username) != 0) {
            fwrite(&cartItem, sizeof(CartItem), 1, fpTemp);
        }
    }

    fclose(fpCart);
    fclose(fpTemp);

    remove("cart.dat");
    rename("temp.dat", "cart.dat");


    