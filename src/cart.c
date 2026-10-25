#include "cart.h"
#include "orders.h"
#include "models.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =================================================================
// 1. ADD TO CART FUNCTION
// =================================================================

void addToCart(const char *username, int productId, int quantity)
{
    Product product;
    int found = 0;
    char line[512];

    if (username == NULL || username[0] == '\0' || quantity <= 0)
    {
        printf("Invalid username or quantity.\n");
        return;
    }

    /* Find product */
    FILE *fpProd = fopen("data/products.txt", "r");

    if (fpProd == NULL)
    {
        printf("Error: Cannot open products.txt\n");
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
                   product.description) == 6)
        {
            if (product.productId == productId)
            {
                found = 1;
                break;
            }
        }
    }

    fclose(fpProd);

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
        printf("Invalid quantity. Available stock: %d\n",
               product.stockQuantity);
        return;
    }

    /*
     * Read existing cart records.
     * If this user already has the product,
     * combine the quantities instead of adding a duplicate.
     */

    FILE *fpCart = fopen("data/cart.txt", "r");
    FILE *fpTemp = fopen("data/temp_cart.txt", "w");

    if (fpTemp == NULL)
    {
        if (fpCart != NULL)
            fclose(fpCart);

        printf("Error: Cannot create temporary cart file.\n");
        return;
    }

    int existingQuantity = 0;
    int cartError = 0;
    int merged = 0;

    if (fpCart != NULL)
    {
        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char savedUsername[100];
            int savedProductId;
            char savedProductName[100];
            int savedQuantity;
            float savedPrice, savedSubtotal;

            if (sscanf(line, "%99[^|]|%d|%99[^|]|%d|%f|%f",
                       savedUsername,
                       &savedProductId,
                       savedProductName,
                       &savedQuantity,
                       &savedPrice,
                       &savedSubtotal) != 6)
            {
                if (fputs(line, fpTemp) == EOF)
                    cartError = 1;

                continue;
            }

            if (strcmp(savedUsername, username) == 0 &&
                savedProductId == productId)
            {
                if (merged == 0)
                {
                    existingQuantity += savedQuantity;
                    merged = 1;
                }
                else
                {
                    /* Combine any older duplicate records too. */
                    existingQuantity += savedQuantity;
                }
            }
            else
            {
                if (fputs(line, fpTemp) == EOF)
                    cartError = 1;
            }
        }

        if (ferror(fpCart))
            cartError = 1;

        fclose(fpCart);
    }

    if (cartError)
    {
        fclose(fpTemp);
        remove("data/temp_cart.txt");
        printf("Error reading cart file. Cart was not updated.\n");
        return;
    }

    int totalQuantity = existingQuantity + quantity;

    if (totalQuantity > product.stockQuantity)
    {
        fclose(fpTemp);
        remove("data/temp_cart.txt");

        printf("Cannot add quantity. Available stock: %d\n",
               product.stockQuantity);
        printf("Already in cart: %d\n", existingQuantity);
        return;
    }

    /* Write one combined record for this product. */
    if (fprintf(fpTemp, "%s|%d|%s|%d|%.2f|%.2f\n",
                username,
                product.productId,
                product.productName,
                totalQuantity,
                product.price,
                totalQuantity * product.price) < 0)
    {
        cartError = 1;
    }

    if (fclose(fpTemp) != 0)
        cartError = 1;

    if (cartError)
    {
        remove("data/temp_cart.txt");
        printf("Error saving cart.\n");
        return;
    }

    /*
     * Replace cart file.
     * Keep the original file if replacement fails.
     */
    if (remove("data/cart.txt") != 0 && existingQuantity > 0)
    {
        remove("data/temp_cart.txt");
        printf("Error replacing cart file.\n");
        return;
    }

    if (rename("data/temp_cart.txt", "data/cart.txt") != 0)
    {
        printf("Error updating cart file.\n");
        return;
    }

    printf("\n%s added to cart successfully!\n",
           product.productName);
    printf("Total quantity in cart: %d\n", totalQuantity);
    printf("Subtotal: Rs. %.2f\n", totalQuantity * product.price);
}


// =================================================================
// 2. VIEW CART FUNCTION
// =================================================================

void viewCart(const char *username)
{
    char line[512];
    float totalAmount;
    int choice;

    do
    {
        system("cls");

        printf("\n===============================================================\n");
        printf("                          SHOPPING CART                        \n");
        printf("===============================================================\n");

        printf("%-8s %-20s %-8s %-12s %-12s\n",
               "ID", "Product", "Qty", "Price", "Subtotal");

        printf("---------------------------------------------------------------\n");

        FILE *fpCart = fopen("data/cart.txt", "r");

        int hasItems = 0;
        totalAmount = 0.0f;

        if (fpCart == NULL)
        {
            printf("Error: Cannot open data/cart.txt\n");
            printf("\nPress Enter to return...");

            while (getchar() != '\n')
                ;

            getchar();
            return;
        }

        while (fgets(line, sizeof(line), fpCart) != NULL)
        {
            char cname[50];
            char pname[50];

            int pid;
            int qty;

            float unit;
            float sub;

            if (sscanf(line, "%49[^|]|%d|%49[^|]|%d|%f|%f",
                       cname,
                       &pid,
                       pname,
                       &qty,
                       &unit,
                       &sub) == 6)
            {
                if (strcmp(cname, username) == 0)
                {
                    hasItems = 1;

                    printf("%-8d %-20s %-8d Rs. %-8.2f Rs. %-8.2f\n",
                           pid, pname, qty, unit, sub);

                    totalAmount += sub;
                }
            }
        }

        fclose(fpCart);

        if (!hasItems)
        {
            printf("Your cart is empty.\n");
            printf("===============================================================\n");
            printf("\nPress Enter to return...");

            while (getchar() != '\n')
                ;

            getchar();
            return;
        }

        printf("---------------------------------------------------------------\n");

        printf("%-42s Rs. %.2f\n",
               "Total Amount:", totalAmount);

        printf("===============================================================\n");

        printf("\n1. Update Quantity");
        printf("\n2. Remove Item");
        printf("\n3. Clear Cart");
        printf("\n4. Checkout");
        printf("\n5. Return");
        printf("\nEnter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input!\n");

            while (getchar() != '\n')
                ;

            getchar();
            choice = 0;
            continue;
        }

        int targetId;

        switch (choice)
        {
        case 1:
            updateCartQuantity(username);
            break;

        case 2:
            printf("Enter Product ID to remove: ");

            if (scanf("%d", &targetId) == 1)
            {
                removeFromCart(username, targetId);
            }
            else
            {
                printf("Invalid Product ID!\n");

                while (getchar() != '\n')
                    ;

                getchar();
            }
            break;

        case 3:
            clearCart(username);
            break;

        case 4:
            checkout(username);
            printf("\nPress Enter to continue...");
            getchar();
            break;

        case 5:
            return;

        default:
            printf("Invalid choice!\n");

            while (getchar() != '\n')
                ;

            getchar();
            break;
        }

    } while (1);
}

// =================================================================
// 3. UPDATE QUANTITY FUNCTION
// =================================================================

void updateCartQuantity(const char *username)
{
    int productId;
    int quantity;
    int found = 0;
    int productFound = 0;

    char line[512];

    Product product;

    /* Get Product ID */
    printf("\nEnter Product ID to update quantity: ");

    if (scanf("%d", &productId) != 1)
    {
        printf("Invalid Product ID!\n");

        while (getchar() != '\n')
            ;

        return;
    }

    /* Open products.txt */
    FILE *fpProd = fopen("data/products.txt", "r");

    if (fpProd == NULL)
    {
        printf("Error: Cannot open data/products.txt\n");
        return;
    }

    /* Search product using text file */
    while (fgets(line, sizeof(line), fpProd) != NULL)
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &product.productId,
                   product.productName,
                   product.category,
                   &product.price,
                   &product.stockQuantity,
                   product.description) == 6)
        {
            if (product.productId == productId)
            {
                productFound = 1;
                break;
            }
        }
    }

    fclose(fpProd);

    /* Check product availability */
    if (!productFound)
    {
        printf("Error: Product ID not found in inventory!\n");
        return;
    }

    /* Get new quantity */
    printf("Enter new quantity (Available Stock: %d): ",
           product.stockQuantity);

    if (scanf("%d", &quantity) != 1)
    {
        printf("Invalid quantity!\n");

        while (getchar() != '\n')
            ;

        return;
    }

    if (quantity <= 0 || quantity > product.stockQuantity)
    {
        printf("Invalid quantity or exceeds available stock!\n");
        return;
    }

    /* Read cart records into memory */
    FILE *fpCart = fopen("data/cart.txt", "r");

    if (fpCart == NULL)
    {
        printf("Error: Cannot open data/cart.txt\n");
        return;
    }

    char **records = NULL;
    int recordCount = 0;
    int capacity = 10;

    records = malloc(capacity * sizeof(char *));

    if (records == NULL)
    {
        printf("Memory allocation failed!\n");
        fclose(fpCart);
        return;
    }

    while (fgets(line, sizeof(line), fpCart) != NULL)
    {
        char cname[50];
        char pname[50];

        int pid;
        int qty;

        float unit;
        float subtotal;

        if (sscanf(line, "%49[^|]|%d|%49[^|]|%d|%f|%f",
                   cname, &pid, pname, &qty, &unit, &subtotal) == 6)
        {
            if (strcmp(cname, username) == 0 && pid == productId)
            {
                qty = quantity;
                subtotal = quantity * unit;
                found = 1;

                snprintf(line, sizeof(line),
                         "%s|%d|%s|%d|%.2f|%.2f\n",
                         cname, pid, pname, qty, unit, subtotal);
            }
        }

        if (recordCount == capacity)
        {
            capacity *= 2;

            char **newRecords = realloc(
                records, capacity * sizeof(char *));

            if (newRecords == NULL)
            {
                printf("Memory allocation failed!\n");

                for (int i = 0; i < recordCount; i++)
                    free(records[i]);

                free(records);
                fclose(fpCart);
                return;
            }

            records = newRecords;
        }

        records[recordCount] = malloc(strlen(line) + 1);

        if (records[recordCount] == NULL)
        {
            printf("Memory allocation failed!\n");

            for (int i = 0; i < recordCount; i++)
                free(records[i]);

            free(records);
            fclose(fpCart);
            return;
        }

        strcpy(records[recordCount], line);
        recordCount++;
    }

    fclose(fpCart);

    /* Check whether item exists in the user's cart */
    if (!found)
    {
        printf("Error: Product ID %d not found in your cart!\n",
               productId);

        for (int i = 0; i < recordCount; i++)
            free(records[i]);

        free(records);
        return;
    }

    /* Rewrite cart.txt */
    fpCart = fopen("data/cart.txt", "w");

    if (fpCart == NULL)
    {
        printf("Error: Cannot rewrite data/cart.txt\n");

        for (int i = 0; i < recordCount; i++)
            free(records[i]);

        free(records);
        return;
    }

    for (int i = 0; i < recordCount; i++)
    {
        fputs(records[i], fpCart);
        free(records[i]);
    }

    free(records);
    fclose(fpCart);

    printf("\nCart quantity updated successfully!\n");
    printf("New quantity: %d\n", quantity);
    printf("New subtotal: Rs. %.2f\n",
           quantity * product.price);
}

// =================================================================
// 4. REMOVE ITEM FUNCTION
// =================================================================
void removeFromCart(const char *username, int productId)
{
    char line[512];
    int found = 0;

    FILE *fpCart = fopen("data/cart.txt", "r");

    if (fpCart == NULL)
    {
        printf("[Error] Could not open data/cart.txt!\n");
        return;
    }

    FILE *fpTemp = fopen("data/temp_cart.txt", "w");

    if (fpTemp == NULL)
    {
        printf("[Error] Could not create temporary file!\n");
        fclose(fpCart);
        return;
    }

    while (fgets(line, sizeof(line), fpCart) != NULL)
    {
        char cname[50];
        char pname[50];
        int pid;
        int qty;
        float unit;
        float subtotal;

        if (sscanf(line, "%49[^|]|%d|%49[^|]|%d|%f|%f",
                   cname, &pid, pname, &qty, &unit, &subtotal) == 6)
        {
            if (strcmp(cname, username) == 0 && pid == productId)
            {
                found = 1;
                continue;
            }
        }

        fputs(line, fpTemp);
    }

    fclose(fpCart);

    if (fclose(fpTemp) != 0)
    {
        printf("[Error] Could not finish writing temporary file!\n");
        return;
    }

    if (!found)
    {
        remove("data/temp_cart.txt");

        printf("[Error] Product ID %d not found in your cart!\n",
               productId);
        return;
    }

    if (remove("data/cart.txt") != 0)
    {
        printf("[Error] Could not replace cart file!\n");
        return;
    }

    if (rename("data/temp_cart.txt", "data/cart.txt") != 0)
    {
        printf("[Error] Could not rename temporary cart file!\n");
        return;
    }

    printf("[Success] Product ID %d removed from your cart.\n",
           productId);

    printf("Press Enter to continue...");
    while (getchar() != '\n')
        ;
    getchar();
}
// =================================================================
// 5. CLEAR CART FUNCTION
// =================================================================

void clearCart(const char *username)
{
    if (username == NULL || username[0] == '\0')
    {
        printf("Error: Invalid username.\n");
        return;
    }

    FILE *source = fopen("data/cart.txt", "r");

    if (source == NULL)
    {
        printf("Cart file could not be opened.\n");
        return;
    }

    FILE *temp = fopen("data/temp_cart.txt", "w");

    if (temp == NULL)
    {
        fclose(source);
        printf("Error: Could not create temporary cart file.\n");
        return;
    }

    char line[512];
    int error = 0;

    while (fgets(line, sizeof(line), source) != NULL)
    {
        char savedUsername[30];

        if (sscanf(line, "%29[^|]", savedUsername) != 1)
        {
            if (fputs(line, temp) == EOF)
                error = 1;

            continue;
        }

        /* Preserve other users' cart items */
        if (strcmp(savedUsername, username) != 0)
        {
            if (fputs(line, temp) == EOF)
                error = 1;
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
        remove("data/temp_cart.txt");
        printf("Error: Could not update cart file.\n");
        return;
    }

    if (remove("data/cart.txt") != 0)
    {
        remove("data/temp_cart.txt");
        printf("Error: Could not replace cart file.\n");
        return;
    }

    if (rename("data/temp_cart.txt", "data/cart.txt") != 0)
    {
        printf("Error: Could not rename temporary cart file.\n");
        return;
    }

    printf("Cart cleared successfully!\n");
}
