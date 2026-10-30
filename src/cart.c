#include "cart.h"
#include "orders.h"
#include "models.h"
#include "input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PRODUCTS_FILE "data/products.txt"
#define CART_FILE "data/cart.txt"
#define CART_TEMP_FILE "data/temp_cart.txt"
#define CART_BACKUP_FILE "data/cart_backup.txt"

/* Wait for a complete line without leaving a stray newline in stdin. */
static void pauseForEnter(const char *message)
{
    char buffer[8];
    (void)readLine(message, buffer, sizeof(buffer));
}

/*
 * Replace cart.txt safely on platforms where rename() cannot overwrite an
 * existing destination (notably Windows). The original is first moved to a
 * backup; if installing the temporary file fails, the backup is restored.
 * All files are in the same directory to avoid cross-filesystem renames.
 */
static int replaceCartFile(void)
{
    FILE *check;
    int originalExists = 0;

    check = fopen(CART_FILE, "r");
    if (check != NULL)
    {
        originalExists = 1;
        fclose(check);
    }

    if (!originalExists)
    {
        if (rename(CART_TEMP_FILE, CART_FILE) != 0)
        {
            printf("Error: Could not install the updated cart file.\n");
            return 0;
        }
        return 1;
    }

    /* Do not overwrite a backup that might contain recoverable data. */
    check = fopen(CART_BACKUP_FILE, "r");
    if (check != NULL)
    {
        fclose(check);
        printf("Error: %s already exists. Please inspect it before retrying.\n",
               CART_BACKUP_FILE);
        return 0;
    }

    if (rename(CART_FILE, CART_BACKUP_FILE) != 0)
    {
        printf("Error: Could not protect the original cart file.\n");
        return 0;
    }

    if (rename(CART_TEMP_FILE, CART_FILE) != 0)
    {
        printf("Error: Could not install the updated cart file.\n");
        if (rename(CART_BACKUP_FILE, CART_FILE) != 0)
        {
            printf("CRITICAL: Restore failed. Original data remains in %s.\n",
                   CART_BACKUP_FILE);
        }
        return 0;
    }

    if (remove(CART_BACKUP_FILE) != 0)
    {
        printf("Warning: Cart updated, but backup remains at %s.\n",
               CART_BACKUP_FILE);
    }

    return 1;
}

// =================================================================
// 1. ADD TO CART FUNCTION
// =================================================================

void addToCart(const char *username, int productId, int quantity)
{
    Product product;
    int found = 0;
    char line[512];
    FILE *fpProd;
    FILE *fpCart;
    FILE *fpTemp;
    int existingQuantity = 0;
    int cartError = 0;

    if (username == NULL || username[0] == '\0' || productId <= 0 || quantity <= 0)
    {
        printf("Invalid username, product ID, or quantity.\n");
        return;
    }

    fpProd = fopen(PRODUCTS_FILE, "r");
    if (fpProd == NULL)
    {
        printf("Error: Cannot open %s\n", PRODUCTS_FILE);
        return;
    }

    while (fgets(line, sizeof(line), fpProd) != NULL)
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &product.productId,
                   product.productName,
                   product.category,
                   &product.price,
                   &product.stockQuantity,
                   product.description) == 6 && product.productId == productId)
        {
            found = 1;
            break;
        }
    }

    if (ferror(fpProd))
        cartError = 1;
    if (fclose(fpProd) != 0)
        cartError = 1;
    if (cartError)
    {
        printf("Error reading product inventory. Cart was not updated.\n");
        return;
    }

    if (!found)
    {
        printf("Error: Product ID %d not found.\n", productId);
        return;
    }
    if (product.stockQuantity <= 0)
    {
        printf("Sorry! This product is out of stock.\n");
        return;
    }
    if (quantity > product.stockQuantity)
    {
        printf("Invalid quantity. Available stock: %d\n", product.stockQuantity);
        return;
    }

    fpCart = fopen(CART_FILE, "r");
    /* If cart.txt does not exist yet, create a new cart through the temp file. */
    fpTemp = fopen(CART_TEMP_FILE, "w");
    if (fpTemp == NULL)
    {
        if (fpCart != NULL)
            fclose(fpCart);
        printf("Error: Cannot create temporary cart file.\n");
        return;
    }

    if (fpCart != NULL)
    {
        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char savedUsername[100];
            char savedProductName[100];
            int savedProductId, savedQuantity;
            float savedPrice, savedSubtotal;

            if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                       savedUsername, &savedProductId, savedProductName,
                       &savedQuantity, &savedPrice, &savedSubtotal) != 6)
            {
                if (fputs(line, fpTemp) == EOF)
                    cartError = 1;
                continue;
            }

            if (strcmp(savedUsername, username) == 0 && savedProductId == productId)
            {
                if (savedQuantity > 0 && existingQuantity <= INT_MAX - savedQuantity)
                    existingQuantity += savedQuantity;
                else
                    cartError = 1;
            }
            else if (fputs(line, fpTemp) == EOF)
            {
                cartError = 1;
            }
        }

        if (ferror(fpCart))
            cartError = 1;
        if (fclose(fpCart) != 0)
            cartError = 1;
    }

    if (cartError)
    {
        fclose(fpTemp);
        remove(CART_TEMP_FILE);
        printf("Error reading or processing cart file. Cart was not updated.\n");
        return;
    }

    if (existingQuantity > INT_MAX - quantity)
    {
        fclose(fpTemp);
        remove(CART_TEMP_FILE);
        printf("Quantity is too large.\n");
        return;
    }

    {
        int totalQuantity = existingQuantity + quantity;
        if (totalQuantity > product.stockQuantity)
        {
            fclose(fpTemp);
            remove(CART_TEMP_FILE);
            printf("Cannot add quantity. Available stock: %d\n", product.stockQuantity);
            printf("Already in cart: %d\n", existingQuantity);
            return;
        }

        if (fprintf(fpTemp, "%s|%d|%s|%d|%.2f|%.2f\n",
                    username, product.productId, product.productName,
                    totalQuantity, product.price, totalQuantity * product.price) < 0)
            cartError = 1;

        if (fclose(fpTemp) != 0)
            cartError = 1;

        if (cartError)
        {
            remove(CART_TEMP_FILE);
            printf("Error saving cart. Original cart was not changed.\n");
            return;
        }

        if (!replaceCartFile())
        {
            remove(CART_TEMP_FILE);
            return;
        }

        printf("\n%s added to cart successfully!\n", product.productName);
        printf("Total quantity in cart: %d\n", totalQuantity);
        printf("Subtotal: Rs. %.2f\n", totalQuantity * product.price);
    }
}

// =================================================================
// 2. VIEW CART FUNCTION
// =================================================================

void viewCart(const char *username)
{
    char line[512];
    int choice;

    if (username == NULL || username[0] == '\0')
    {
        printf("Error: Invalid username.\n");
        return;
    }

    for (;;)
    {
        float totalAmount = 0.0f;
        int hasItems = 0;
        FILE *fpCart;

#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        printf("\n===============================================================\n");
        printf("                          SHOPPING CART                        \n");
        printf("===============================================================\n");
        printf("%-8s %-20s %-8s %-12s %-12s\n",
               "ID", "Product", "Qty", "Price", "Subtotal");
        printf("---------------------------------------------------------------\n");

        fpCart = fopen(CART_FILE, "r");
        if (fpCart == NULL)
        {
            printf("Your cart is empty or the cart file is unavailable.\n");
            pauseForEnter("Press Enter to return...");
            return;
        }

        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char cname[100], pname[100];
            int pid, qty;
            float unit, subtotal;

            if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                       cname, &pid, pname, &qty, &unit, &subtotal) == 6 &&
                strcmp(cname, username) == 0)
            {
                hasItems = 1;
                printf("%-8d %-20.20s %-8d Rs. %-8.2f Rs. %-8.2f\n",
                       pid, pname, qty, unit, subtotal);
                totalAmount += subtotal;
            }
        }

        if (ferror(fpCart))
        {
            fclose(fpCart);
            printf("Error reading cart file.\n");
            pauseForEnter("Press Enter to return...");
            return;
        }
        fclose(fpCart);

        if (!hasItems)
        {
            printf("Your cart is empty.\n");
            pauseForEnter("Press Enter to return...");
            return;
        }

        printf("---------------------------------------------------------------\n");
        printf("%-42s Rs. %.2f\n", "Total Amount:", totalAmount);
        printf("===============================================================\n");
        printf("\n1. Update Quantity\n2. Remove Item\n3. Clear Cart\n4. Checkout\n5. Return\n");

        if (!readInteger("Enter choice: ", &choice, 1, 5))
            return;

        switch (choice)
        {
        case 1:
            updateCartQuantity(username);
            pauseForEnter("Press Enter to continue...");
            break;
        case 2:
        {
            int targetId;
            if (readInteger("Enter Product ID to remove: ", &targetId, 1, INT_MAX))
                removeFromCart(username, targetId);
            pauseForEnter("Press Enter to continue...");
            break;
        }
        case 3:
            clearCart(username);
            pauseForEnter("Press Enter to continue...");
            break;
        case 4:
            checkout(username);
            pauseForEnter("Press Enter to continue...");
            break;
        case 5:
            return;
        default:
            /* readInteger constrains the choice to 1..5. */
            break;
        }
    }
}

// =================================================================
// 3. UPDATE QUANTITY FUNCTION
// =================================================================

void updateCartQuantity(const char *username)
{
    int productId, quantity;
    int found = 0, productFound = 0;
    int error = 0;
    char line[512];
    Product product;
    FILE *fpProd;
    FILE *fpCart;
    FILE *fpTemp;

    if (username == NULL || username[0] == '\0')
    {
        printf("Error: Invalid username.\n");
        return;
    }

    if (!readInteger("Enter Product ID to update quantity: ",
                     &productId, 1, INT_MAX))
        return;

    fpProd = fopen(PRODUCTS_FILE, "r");
    if (fpProd == NULL)
    {
        printf("Error: Cannot open %s\n", PRODUCTS_FILE);
        return;
    }

    while (fgets(line, sizeof(line), fpProd) != NULL)
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &product.productId, product.productName, product.category,
                   &product.price, &product.stockQuantity, product.description) == 6 &&
            product.productId == productId)
        {
            productFound = 1;
            break;
        }
    }

    if (ferror(fpProd))
        error = 1;
    if (fclose(fpProd) != 0)
        error = 1;
    if (error)
    {
        printf("Error reading product inventory.\n");
        return;
    }
    if (!productFound)
    {
        printf("Error: Product ID not found in inventory.\n");
        return;
    }
    if (product.stockQuantity <= 0)
    {
        printf("This product is out of stock.\n");
        return;
    }

    {
        char prompt[120];
        snprintf(prompt, sizeof(prompt), "Enter new quantity (Available Stock: %d): ",
                 product.stockQuantity);
        if (!readInteger(prompt, &quantity, 1, product.stockQuantity))
            return;
    }

    fpCart = fopen(CART_FILE, "r");
    if (fpCart == NULL)
    {
        printf("Error: Cannot open %s\n", CART_FILE);
        return;
    }
    fpTemp = fopen(CART_TEMP_FILE, "w");
    if (fpTemp == NULL)
    {
        fclose(fpCart);
        printf("Error: Cannot create temporary cart file.\n");
        return;
    }

    while (fgets(line, sizeof(line), fpCart) != NULL)
    {
        char cname[100], pname[100];
        int pid, qty;
        float unit, subtotal;

        if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                   cname, &pid, pname, &qty, &unit, &subtotal) == 6 &&
            strcmp(cname, username) == 0 && pid == productId)
        {
            found = 1;
            if (fprintf(fpTemp, "%s|%d|%s|%d|%.2f|%.2f\n",
                        cname, pid, pname, quantity, unit, quantity * unit) < 0)
                error = 1;
        }
        else if (fputs(line, fpTemp) == EOF)
        {
            error = 1;
        }

        if (error)
            break;
    }

    if (ferror(fpCart))
        error = 1;
    if (fclose(fpCart) != 0)
        error = 1;
    if (fclose(fpTemp) != 0)
        error = 1;

    if (error)
    {
        remove(CART_TEMP_FILE);
        printf("Error reading or writing cart. Original cart was not changed.\n");
        return;
    }
    if (!found)
    {
        remove(CART_TEMP_FILE);
        printf("Error: Product ID %d was not found in your cart.\n", productId);
        return;
    }
    if (!replaceCartFile())
    {
        remove(CART_TEMP_FILE);
        return;
    }

    printf("\nCart quantity updated successfully!\n");
    printf("New quantity: %d\n", quantity);
    printf("New subtotal: Rs. %.2f\n", quantity * product.price);
}

// =================================================================
// 4. REMOVE ITEM FUNCTION
// =================================================================

void removeFromCart(const char *username, int productId)
{
    char line[512];
    int found = 0, error = 0;
    FILE *fpCart;
    FILE *fpTemp;

    if (username == NULL || username[0] == '\0' || productId <= 0)
    {
        printf("Error: Invalid username or product ID.\n");
        return;
    }

    fpCart = fopen(CART_FILE, "r");
    if (fpCart == NULL)
    {
        printf("Error: Could not open %s.\n", CART_FILE);
        return;
    }
    fpTemp = fopen(CART_TEMP_FILE, "w");
    if (fpTemp == NULL)
    {
        fclose(fpCart);
        printf("Error: Could not create temporary cart file.\n");
        return;
    }

    while (fgets(line, sizeof(line), fpCart) != NULL)
    {
        char cname[100], pname[100];
        int pid, qty;
        float unit, subtotal;

        if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                   cname, &pid, pname, &qty, &unit, &subtotal) == 6 &&
            strcmp(cname, username) == 0 && pid == productId)
        {
            found = 1;
            continue;
        }

        if (fputs(line, fpTemp) == EOF)
        {
            error = 1;
            break;
        }
    }

    if (ferror(fpCart))
        error = 1;
    if (fclose(fpCart) != 0)
        error = 1;
    if (fclose(fpTemp) != 0)
        error = 1;

    if (error)
    {
        remove(CART_TEMP_FILE);
        printf("Error reading or writing cart. Original cart was not changed.\n");
        return;
    }
    if (!found)
    {
        remove(CART_TEMP_FILE);
        printf("Error: Product ID %d was not found in your cart.\n", productId);
        return;
    }
    if (!replaceCartFile())
    {
        remove(CART_TEMP_FILE);
        return;
    }

    printf("Product ID %d removed from your cart.\n", productId);
}

// =================================================================
// 5. CLEAR CART FUNCTION
// =================================================================

void clearCart(const char *username)
{
    char line[512];
    int error = 0;
    int removedAny = 0;
    FILE *source;
    FILE *temp;

    if (username == NULL || username[0] == '\0')
    {
        printf("Error: Invalid username.\n");
        return;
    }

    source = fopen(CART_FILE, "r");
    if (source == NULL)
    {
        printf("Cart file could not be opened.\n");
        return;
    }
    temp = fopen(CART_TEMP_FILE, "w");
    if (temp == NULL)
    {
        fclose(source);
        printf("Error: Could not create temporary cart file.\n");
        return;
    }

    while (fgets(line, sizeof(line), source) != NULL)
    {
        char savedUsername[100];
        if (sscanf(line, "%99[^|]", savedUsername) == 1 &&
            strcmp(savedUsername, username) == 0)
        {
            removedAny = 1;
            continue;
        }

        if (fputs(line, temp) == EOF)
        {
            error = 1;
            break;
        }
    }

    if (ferror(source))
        error = 1;
    if (fclose(source) != 0)
        error = 1;
    if (fclose(temp) != 0)
        error = 1;

    if (error)
    {
        remove(CART_TEMP_FILE);
        printf("Error reading or writing cart. Original cart was not changed.\n");
        return;
    }
    if (!removedAny)
    {
        remove(CART_TEMP_FILE);
        printf("Your cart is already empty.\n");
        return;
    }
    if (!replaceCartFile())
    {
        remove(CART_TEMP_FILE);
        return;
    }

    printf("Cart cleared successfully!\n");
}
