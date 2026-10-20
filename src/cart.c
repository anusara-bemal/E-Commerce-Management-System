<<<<<<< HEAD
#include<stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct definitions following PascalCase and G14 guidelines
typedef struct {
    int productId;
    char productName[50];
    char category[30];
    float price;
    int stockQuantity;
    char description[100];
} Product;

typedef struct {
    int customerId;
    int productId;
    char productName[50];
    int quantity;
    float unitPrice;
    float subtotal;
} CartItem;

// Function Prototypes following camelCase and G14 guidelines
void addToCart(int customerId);
void viewCart(int customerId);
void updateCartQuantity(int customerId);
void removeFromCart(int customerId);
void clearCart(int customerId);


// 1. ADD PRODUCT TO CART

void addToCart(int customerId) {
    int productId;
    int quantity;
    Product product;
    CartItem cartItem;
    int found = 0;

    printf("\n========================================\n");
    printf("            ADD PRODUCT TO CART         \n");
    printf("========================================\n");
    printf("Enter Product ID to buy: ");
    scanf("%d", &productId);

    // Check if product exists and has enough stock in products.dat
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

    printf("Product Found: %s | Price: Rs. %.2f | Available Stock: %d\n", product.productName, product.price, product.stockQuantity);
    printf("Enter Quantity: ");
    scanf("%d", &quantity);

    if (quantity <= 0 || quantity > product.stockQuantity) {
        printf("[Error] Invalid quantity or exceeds available stock!\n");
        return;
    }

    // Populate CartItem structure using G14 guidelines
    cartItem.customerId = customerId;
    cartItem.productId = product.productId;
    strcpy(cartItem.productName, product.productName);
    cartItem.quantity = quantity;
    cartItem.unitPrice = product.price;
    cartItem.subtotal = quantity * product.price;

    // Save item to cart.dat
    FILE *fpCart = fopen("cart.dat", "ab");
    if (fpCart == NULL) {
        printf("[Error] Could not open cart file for writing!\n");
        return;
    }

    fwrite(&cartItem, sizeof(CartItem), 1, fpCart);
    fclose(fpCart);

    printf("\n[Success] %s (Qty: %d) added to your cart successfully!\n", cartItem.productName, cartItem.quantity);
}


// 2. VIEW CART & CART OPTIONS MENU

void viewCart(int customerId) {
    CartItem cartItem;
    float totalAmount = 0.0;
    int choice = 5;

    do {
        system("clear"); // Use "cls" if compiling on Windows
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
                if (cartItem.customerId == customerId) {
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
            while (getchar() != '\n'); // Clear buffer
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

        switch (choice) {
            case 1:
                updateCartQuantity(customerId);
                break;
            case 2:
                removeFromCart(customerId);
                break;
            case 3:
                clearCart(customerId);
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


// 3. UPDATE CART QUANTITY

void updateCartQuantity(int customerId) {
    int productId;
    int quantity;
    int found = 0;

    printf("\nEnter Product ID to update quantity: ");
    scanf("%d", &productId);

    // Check available stock in products.dat
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
        if (cartItem.customerId == customerId && cartItem.productId == productId) {
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

// -----------------------------------------------------------------
// 4. REMOVE ITEM FROM CART
// -----------------------------------------------------------------
void removeFromCart(int customerId) {
    int productId;
    int found = 0;

    printf("\nEnter Product ID to remove from cart: ");
    scanf("%d", &productId);

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
        if (cartItem.customerId == customerId && cartItem.productId == productId) {
            found = 1; // Skip this item to remove it
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

// 5. CLEAR CART

void clearCart(int customerId) {
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
        // Keep items belonging to other customers, remove for current customer
        if (cartItem.customerId != customerId) {
            fwrite(&cartItem, sizeof(CartItem), 1, fpTemp);
        }
    }

    fclose(fpCart);
    fclose(fpTemp);

    remove("cart.dat");
    rename("temp.dat", "cart.dat");

    printf("[Success] Your cart has been cleared successfully!\n");
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    getchar();
}
=======
#include "../include/cart.h"
#include <stdio.h>

void viewCart(char* username) {
    printf("Cart for user: %s\n", username);
    // TODO: උඹේ logic එක මෙතන ලියන්න - data/cart.dat read කරන එක
    printf("Cart feature coming soon - abyss level implementation pending!\n");
}

void addToCart(char* username, int productId, int quantity) {
    // implementation
}

void removeFromCart(char* username, int productId) {
    // implementation
}

void clearCart(char* username) {
    // implementation
}

//mena idan oyage code eka karagena yanna
>>>>>>> 2d388b57d2b499a0c52c3e49928c7b30648e964a
