#include "cart.h"
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

    FILE *fpProd = fopen("data/products.txt", "r");

    if (fpProd == NULL)
    {
        printf("Error: Cannot open products.txt\n");
        return;
    }

    /* Find product using Product ID */
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

    /* Check whether product exists */
    if (!found)
    {
        printf("Error: Product ID %d not found.\n", productId);
        return;
    }

    /* Check available stock */
    if (product.stockQuantity <= 0)
    {
        printf("Sorry! This product is out of stock.\n");
        return;
    }

    /* Validate quantity */
    if (quantity <= 0 || quantity > product.stockQuantity)
    {
        printf("Invalid quantity. Available stock: %d\n",
               product.stockQuantity);
        return;
    }

    /* Open cart.txt in append mode */
    FILE *fpCart = fopen("data/cart.txt", "a");

    if (fpCart == NULL)
    {
        printf("Error: Cannot open cart.txt\n");
        return;
    }

    /* Save cart item as a text record */
    fprintf(fpCart, "%s|%d|%s|%d|%.2f|%.2f\n",
            username,
            product.productId,
            product.productName,
            quantity,
            product.price,
            quantity * product.price);

    fclose(fpCart);

    printf("\n%s added to cart successfully!\n",
           product.productName);

    printf("Quantity: %d\n", quantity);
    printf("Subtotal: Rs. %.2f\n", quantity * product.price);
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
                printf("Checkout functionality will be implemented separately.\n");

                printf("\nPress Enter to continue...");

                while (getchar() != '\n')
                    ;

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
    char line[512];
    int found = 0;

    FILE *fpCart = fopen("data/cart.txt", "r");

    if (fpCart == NULL)
    {
        printf("[Error] Could not open data/cart.txt!\n");
        return;
    }

    char **records = NULL;
    int recordCount = 0;
    int capacity = 10;

    records = malloc(capacity * sizeof(char *));

    if (records == NULL)
    {
        printf("[Error] Memory allocation failed!\n");
        fclose(fpCart);
        return;
    }

    /* Read cart records from the text file */
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
            /* Skip items belonging to the current user */
            if (strcmp(cname, username) == 0)
            {
                found = 1;
                continue;
            }
        }

        /* Increase memory capacity if necessary */
        if (recordCount == capacity)
        {
            capacity *= 2;

            char **newRecords = realloc(
                records, capacity * sizeof(char *));

            if (newRecords == NULL)
            {
                printf("[Error] Memory allocation failed!\n");

                for (int i = 0; i < recordCount; i++)
                {
                    free(records[i]);
                }

                free(records);
                fclose(fpCart);
                return;
            }

            records = newRecords;
        }

        records[recordCount] = malloc(strlen(line) + 1);

        if (records[recordCount] == NULL)
        {
            printf("[Error] Memory allocation failed!\n");

            for (int i = 0; i < recordCount; i++)
            {
                free(records[i]);
            }

            free(records);
            fclose(fpCart);
            return;
        }

        strcpy(records[recordCount], line);
        recordCount++;
    }

    fclose(fpCart);

    /* Check whether the user has any cart items */
    if (!found)
    {
        printf("[Info] Your cart is already empty.\n");

        for (int i = 0; i < recordCount; i++)
        {
            free(records[i]);
        }

        free(records);
        return;
    }

    /* Rewrite cart.txt without the current user's items */
    fpCart = fopen("data/cart.txt", "w");

    if (fpCart == NULL)
    {
        printf("[Error] Could not rewrite data/cart.txt!\n");

        for (int i = 0; i < recordCount; i++)
        {
            free(records[i]);
        }

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

    printf("[Success] Your cart has been cleared!\n");
}



    