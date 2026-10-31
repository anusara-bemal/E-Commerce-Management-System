#include "cart.h"
#include "orders.h"
#include "models.h"
#include "input.h"
#include "ui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define PRODUCTS_FILE "data/product.txt"
#define CART_FILE "data/cart.txt"
#define CART_TEMP_FILE "data/temp_cart.txt"
#define CART_BACKUP_FILE "data/cart_backup.txt"

/* Wait for a complete line without leaving a stray newline in stdin. */
// static void pauseForEnter(const char *message)
// {
//     char buffer[8];
//     (void)readLine(message, buffer, sizeof(buffer));
// }

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
        showError("Invalid username, product ID, or quantity.");
        return;
    }

    fpProd = fopen(PRODUCTS_FILE, "r");
    if (fpProd == NULL)
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "Cannot open %s", PRODUCTS_FILE);
        showError(msg);
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

    if (ferror(fpProd)) cartError = 1;
    if (fclose(fpProd) != 0) cartError = 1;
    if (cartError)
    {
        showError("Error reading product inventory. Cart was not updated.");
        return;
    }

    if (!found)
    {
        char msg[80];
        snprintf(msg, sizeof(msg), "Product ID %d not found.", productId);
        showError(msg);
        return;
    }
    if (product.stockQuantity <= 0)
    {
        showError("Sorry! This product is out of stock.");
        return;
    }
    if (quantity > product.stockQuantity)
    {
        char msg[80];
        snprintf(msg, sizeof(msg), "Invalid quantity. Available stock: %d", product.stockQuantity);
        showWarning(msg);
        return;
    }

    fpCart = fopen(CART_FILE, "r");
    fpTemp = fopen(CART_TEMP_FILE, "w");
    if (fpTemp == NULL)
    {
        if (fpCart != NULL) fclose(fpCart);
        showError("Cannot create temporary cart file.");
        return;
    }

    if (fpCart != NULL)
    {
        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char savedUsername[100], savedProductName[100];
            int savedProductId, savedQuantity;
            float savedPrice, savedSubtotal;

            if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                       savedUsername, &savedProductId, savedProductName,
                       &savedQuantity, &savedPrice, &savedSubtotal) != 6)
            {
                if (fputs(line, fpTemp) == EOF) cartError = 1;
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
        if (ferror(fpCart)) cartError = 1;
        if (fclose(fpCart) != 0) cartError = 1;
    }

    if (cartError)
    {
        fclose(fpTemp);
        remove(CART_TEMP_FILE);
        showError("Error reading or processing cart file. Cart was not updated.");
        return;
    }

    if (existingQuantity > INT_MAX - quantity)
    {
        fclose(fpTemp);
        remove(CART_TEMP_FILE);
        showError("Quantity is too large.");
        return;
    }

    {
        int totalQuantity = existingQuantity + quantity;
        if (totalQuantity > product.stockQuantity)
        {
            fclose(fpTemp);
            remove(CART_TEMP_FILE);
            char msg[120];
            snprintf(msg, sizeof(msg), "Cannot add. Stock: %d | Already in cart: %d", product.stockQuantity, existingQuantity);
            showWarning(msg);
            return;
        }

        if (fprintf(fpTemp, "%s|%d|%s|%d|%.2f|%.2f\n",
                    username, product.productId, product.productName,
                    totalQuantity, product.price, totalQuantity * product.price) < 0)
            cartError = 1;

        if (fclose(fpTemp) != 0) cartError = 1;

        if (cartError)
        {
            remove(CART_TEMP_FILE);
            showError("Error saving cart. Original cart was not changed.");
            return;
        }

        if (!replaceCartFile())
        {
            remove(CART_TEMP_FILE);
            return;
        }

        // SUCCESS UI
        printf("\n");
        char successMsg[120];
        snprintf(successMsg, sizeof(successMsg), "%s added to cart!", product.productName);
        showSuccess(successMsg);
        printf("\n");
        printf("  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
        printf("  " MAGENTA "│" RESET "  Product : " CYAN "%-32.32s" RESET " " MAGENTA "│\n" RESET, product.productName);
        printf("  " MAGENTA "│" RESET "  Quantity: " WHITE "%-4d" RESET " (Total in cart: " GREEN "%d" RESET ")      " MAGENTA "│\n" RESET, quantity, totalQuantity);
        printf("  " MAGENTA "│" RESET "  Subtotal: " BOLD GREEN "Rs. %-8.2f" RESET "                      " MAGENTA "│\n" RESET, totalQuantity * product.price);
        printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);
    }
}// =================================================================
// 2. VIEW CART FUNCTION
// =================================================================
void viewCart(const char *username)
{
    char line[512];
    int choice;

    if (username == NULL || username[0] == '\0')
    {
        showError("Invalid username.");
        pressEnterToContinue();
        return;
    }

    for (;;)
    {
        float totalAmount = 0.0f;
        int hasItems = 0;
        FILE *fpCart;

        clearScreen();
        drawHeader("SHOPPING CART");

        // Table
        printf("\n");
        printTableHeader(5, "ID", "Product", "Qty", "Price", "Subtotal");
        printTableDivider(5);

        fpCart = fopen(CART_FILE, "r");
        if (fpCart == NULL)
        {
            printTableDivider(5);
            printf("\n");
            showInfo("Your cart is empty or the cart file is unavailable.");
            pressEnterToContinue();
            return;
        }

        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char cname[100], pname[100];
            int pid, qty;
            float unit, subtotal;
            char idStr[10], qtyStr[10], priceStr[20], subStr[20], nameShort[21];

            if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                       cname, &pid, pname, &qty, &unit, &subtotal) == 6 &&
                strcmp(cname, username) == 0)
            {
                hasItems = 1;
                snprintf(idStr, sizeof(idStr), "%d", pid);
                snprintf(nameShort, sizeof(nameShort), "%.20s", pname);
                snprintf(qtyStr, sizeof(qtyStr), "%d", qty);
                snprintf(priceStr, sizeof(priceStr), "Rs.%.2f", unit);
                snprintf(subStr, sizeof(subStr), "Rs.%.2f", subtotal);
                printTableRow(5, idStr, nameShort, qtyStr, priceStr, subStr);
                totalAmount += subtotal;
            }
        }

        if (ferror(fpCart))
        {
            fclose(fpCart);
            printf("\n");
            showError("Error reading cart file.");
            pressEnterToContinue();
            return;
        }
        fclose(fpCart);

        if (!hasItems)
        {
            printTableDivider(5);
            printf("\n");
            showInfo("Your cart is empty. Go add some products!");
            pressEnterToContinue();
            return;
        }

        printTableDivider(5);
        
        // Total Card
        printf("\n  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
        printf("  " MAGENTA "│" RESET "  Total Amount: " BOLD GREEN "Rs. %.2f" RESET "                      " MAGENTA "│\n" RESET, totalAmount);
        printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

        // Actions Menu - Nordic style
        printf("\n  " BOLD WHITE "CART ACTIONS\n" RESET);
        printf("  " CYAN "1" RESET ". Update Quantity\n");
        printf("  " CYAN "2" RESET ". Remove Item\n");
        printf("  " YELLOW "3" RESET ". Clear Cart\n");
        printf("  " GREEN "4" RESET ". Checkout " DIM "(proceed to pay)" RESET "\n");
        printf("  " RED "5" RESET ". Return to Menu\n");

        if (!readInteger("\n  " CYAN "❯" RESET " Enter choice: ", &choice, 1, 5))
            return;

        switch (choice)
        {
        case 1:
            updateCartQuantity(username);
            pressEnterToContinue();
            break;
        case 2:
        {
            int targetId;
            if (readInteger("  " CYAN "❯" RESET " Enter Product ID to remove: ", &targetId, 1, INT_MAX))
                removeFromCart(username, targetId);
            pressEnterToContinue();
            break;
        }
        case 3:
            clearCart(username);
            pressEnterToContinue();
            break;
        case 4:
            checkout(username);
            pressEnterToContinue();
            break;
        case 5:
            return;
        default:
            break;
        }
    }
}// =================================================================
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
