#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "products.h"
#include "ui.h"
#include "cart.h"

// Forward declarations - error එක නැති කරන්න

// Product Management Menu
void productMenu(void)
{
    int choice;
    do
    {
        clearScreen();
        drawHeader("ADMIN PRODUCT MANAGEMENT");
        printf("1. Add Product\n");
        printf("2. View Products\n");
        printf("3. Search Product\n");
        printf("4. Update Product\n");
        printf("5. Delete Product\n");
        printf("6. Back\n");
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n')
                ;
            continue;
        }
        getchar();

        switch (choice)
        {
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
            printf("Returning...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
        if (choice != 6)
        {
            printf("\nPress Enter...");
            getchar();
        }
    } while (choice != 6);
}

void addProduct(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "a");

    if (file == NULL)
    {
        printf("Error opening products file!\n");
        return;
    }

    Product p;

    printf("\n--- Add New Product ---\n");

    printf("Enter Product ID: ");
    if (scanf("%d", &p.productId) != 1)
    {
        printf("Invalid Product ID!\n");
        while (getchar() != '\n' && !feof(stdin))
            ;
        fclose(file);
        return;
    }
    getchar();

    printf("Enter Product Name: ");
    if (fgets(p.productName, sizeof(p.productName), stdin) == NULL)
    {
        fclose(file);
        return;
    }
    p.productName[strcspn(p.productName, "\n")] = '\0';

    printf("Enter Category: ");
    if (fgets(p.category, sizeof(p.category), stdin) == NULL)
    {
        fclose(file);
        return;
    }
    p.category[strcspn(p.category, "\n")] = '\0';

    printf("Enter Price: ");
    if (scanf("%f", &p.price) != 1 || p.price < 0)
    {
        printf("Invalid price!\n");
        while (getchar() != '\n' && !feof(stdin))
            ;
        fclose(file);
        return;
    }

    printf("Enter Stock Quantity: ");
    if (scanf("%d", &p.stockQuantity) != 1 ||
        p.stockQuantity < 0)
    {
        printf("Invalid stock quantity!\n");
        while (getchar() != '\n' && !feof(stdin))
            ;
        fclose(file);
        return;
    }
    getchar();

    printf("Enter Description: ");
    if (fgets(p.description, sizeof(p.description), stdin) == NULL)
    {
        fclose(file);
        return;
    }
    p.description[strcspn(p.description, "\n")] = '\0';

    if (strchr(p.productName, '|') != NULL ||
        strchr(p.category, '|') != NULL ||
        strchr(p.description, '|') != NULL)
    {
        printf("Please do not use the | character in product details.\n");
        fclose(file);
        return;
    }

    if (fprintf(file, "%d|%s|%s|%.2f|%d|%s\n",
                p.productId,
                p.productName,
                p.category,
                p.price,
                p.stockQuantity,
                p.description) < 0)
    {
        printf("Error saving product!\n");
        fclose(file);
        return;
    }

    fclose(file);

    printf("\n%sProduct added successfully!%s\n", GREEN, RESET);
}

void viewProducts(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");

    if (file == NULL)
    {
        printf("No products found! Add products first.\n");
        return;
    }

    Product p;
    char line[512];
    int count = 0;

    printf("\n--------------------------------------------------------------------------\n");
    printf("%-10s %-20s %-15s %-12s %-8s\n",
           "ID", "Name", "Category", "Price", "Stock");
    printf("--------------------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId,
                   p.productName,
                   p.category,
                   &p.price,
                   &p.stockQuantity,
                   p.description) == 6)
        {
            printf("%-10d %-20.20s %-15.15s %-12.2f %-8d\n",
                   p.productId,
                   p.productName,
                   p.category,
                   p.price,
                   p.stockQuantity);

            count++;
        }
        else
        {
            printf("Warning: Invalid product record skipped.\n");
        }
    }

    if (count == 0)
    {
        printf("No valid products found.\n");
    }

    printf("--------------------------------------------------------------------------\n");

    fclose(file);
}

void viewProductsAndAddToCart(const char *username)
{
    int productId;
    int quantity;
    char choice;

    // Display available products
    viewProducts();

    printf("\nDo you want to add a product to cart? (y/n): ");
    if (scanf(" %c", &choice) != 1)
    {
        while (getchar() != '\n' && !feof(stdin))
            ;
        return;
    }

    if (choice != 'y' && choice != 'Y')
    {
        return;
    }

    printf("Enter Product ID: ");
    if (scanf("%d", &productId) != 1)
    {
        printf("Invalid Product ID!\n");
        while (getchar() != '\n' && !feof(stdin))
            ;
        return;
    }

    printf("Enter Quantity: ");
    if (scanf("%d", &quantity) != 1 || quantity <= 0)
    {
        printf("Invalid quantity!\n");
        while (getchar() != '\n' && !feof(stdin))
            ;
        return;
    }

    addToCart(username, productId, quantity);
}

void searchProduct(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");

    if (file == NULL)
    {
        printf("No products available. File 'product.txt' not found!\n");
        return;
    }

    int searchId;
    printf("\nEnter Product ID to search: ");

    if (scanf("%d", &searchId) != 1)
    {
        printf("Invalid Product ID!\n");
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
        fclose(file);
        return;
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

    char line[512];
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strlen(line) < 5)
            continue;

        Product p;
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId,
                   p.productName,
                   p.category,
                   &p.price,
                   &p.stockQuantity,
                   p.description) == 6)
        {
            if (p.productId == searchId)
            {
                printf("\n--- Product Found! ---\n");
                printf("ID          : %d\n", p.productId);
                printf("Name        : %s\n", p.productName);
                printf("Category    : %s\n", p.category);
                printf("Price       : %.2f\n", p.price);
                printf("Stock       : %d\n", p.stockQuantity);
                printf("Description : %s\n", p.description);
                found = 1;
                break;
            }
        }
    }

    if (!found)
    {
        printf("Product ID %d not found.\n", searchId);
    }

    fclose(file);
}

void updateProduct(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");

    if (file == NULL)
    {
        printf("No products to update.\n");
        return;
    }

    FILE *tempFile = fopen("data/temp.txt", "w");

    if (tempFile == NULL)
    {
        printf("Error creating temporary file!\n");
        fclose(file);
        return;
    }

    int searchId;
    int found = 0;
    char line[512];
    char input[100];

    printf("\nEnter Product ID to update: ");

    if (scanf("%d", &searchId) != 1)
    {
        printf("Invalid Product ID!\n");

        while (getchar() != '\n' && !feof(stdin))
            ;

        fclose(file);
        fclose(tempFile);
        remove("data/temp.txt");
        return;
    }

    getchar();

    while (fgets(line, sizeof(line), file) != NULL)
    {
        Product p;

        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId,
                   p.productName,
                   p.category,
                   &p.price,
                   &p.stockQuantity,
                   p.description) != 6)
        {
            printf("Warning: Invalid product record skipped.\n");
            continue;
        }

        if (p.productId == searchId)
        {
            found = 1;

            printf("\nProduct found! Leave a field empty to keep its old value.\n");

            printf("Name [%s]: ", p.productName);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = '\0';

                if (strlen(input) > 0 &&
                    strchr(input, '|') == NULL)
                {
                    strncpy(p.productName, input, sizeof(p.productName) - 1);
                    p.productName[sizeof(p.productName) - 1] = '\0';
                }
                else if (strchr(input, '|') != NULL)
                {
                    printf("The | character is not allowed. Keeping old name.\n");
                }
            }

            printf("Category [%s]: ", p.category);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = '\0';

                if (strlen(input) > 0 &&
                    strchr(input, '|') == NULL)
                {
                    strncpy(p.category, input, sizeof(p.category) - 1);
                    p.category[sizeof(p.category) - 1] = '\0';
                }
                else if (strchr(input, '|') != NULL)
                {
                    printf("The | character is not allowed. Keeping old category.\n");
                }
            }

            printf("Price [%.2f]: ", p.price);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = '\0';

                if (strlen(input) > 0)
                {
                    char *end;
                    float newPrice = strtof(input, &end);

                    if (end != input && *end == '\0' && newPrice >= 0)
                    {
                        p.price = newPrice;
                    }
                    else
                    {
                        printf("Invalid price. Keeping old price.\n");
                    }
                }
            }

            printf("Stock [%d]: ", p.stockQuantity);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = '\0';

                if (strlen(input) > 0)
                {
                    char *end;
                    long newStock = strtol(input, &end, 10);

                    if (end != input && *end == '\0' &&
                        newStock >= 0 && newStock <= 2147483647L)
                    {
                        p.stockQuantity = (int)newStock;
                    }
                    else
                    {
                        printf("Invalid stock quantity. Keeping old stock.\n");
                    }
                }
            }

            printf("Description [%s]: ", p.description);

            if (fgets(input, sizeof(input), stdin) != NULL)
            {
                input[strcspn(input, "\n")] = '\0';

                if (strlen(input) > 0 &&
                    strchr(input, '|') == NULL)
                {
                    snprintf(p.description, sizeof(p.description),
                             "%s", input);
                }
                else if (strchr(input, '|') != NULL)
                {
                    printf("The | character is not allowed. Keeping old description.\n");
                }
            }
        }

        if (fprintf(tempFile, "%d|%s|%s|%.2f|%d|%s\n",
                    p.productId,
                    p.productName,
                    p.category,
                    p.price,
                    p.stockQuantity,
                    p.description) < 0)
        {
            printf("Error writing product data!\n");
            fclose(file);
            fclose(tempFile);
            remove("data/temp.txt");
            return;
        }
    }

    fclose(file);

    if (fclose(tempFile) != 0)
    {
        printf("Error closing temporary file!\n");
        remove("data/temp.txt");
        return;
    }

    if (!found)
    {
        remove("data/temp.txt");
        printf("Product ID %d not found.\n", searchId);
        return;
    }

    if (remove(PRODUCTS_FILE) != 0)
    {
        printf("Error replacing the products file!\n");
        remove("data/temp.txt");
        return;
    }

    if (rename("data/temp.txt", PRODUCTS_FILE) != 0)
    {
        printf("Error saving updated products!\n");
        printf("Your temporary file is data/temp.txt\n");
        return;
    }

    printf("\nProduct updated successfully!\n");
}

void deleteProduct(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");

    if (file == NULL)
    {
        printf("No products to delete.\n");
        return;
    }

    FILE *tempFile = fopen("data/temp.txt", "w");

    if (tempFile == NULL)
    {
        printf("Error creating temporary file!\n");
        fclose(file);
        return;
    }

    int searchId;
    int found = 0;
    char line[512];

    printf("\nEnter Product ID to delete: ");

    if (scanf("%d", &searchId) != 1)
    {
        printf("Invalid Product ID!\n");

        while (getchar() != '\n' && !feof(stdin))
            ;

        fclose(file);
        fclose(tempFile);
        remove("data/temp.txt");
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        Product p;

        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId,
                   p.productName,
                   p.category,
                   &p.price,
                   &p.stockQuantity,
                   p.description) != 6)
        {
            printf("Error: Invalid product record found. Deletion cancelled.\n");
            fclose(file);
            fclose(tempFile);
            remove("data/temp.txt");
            return;
        }

        if (p.productId == searchId)
        {
            found = 1;
            continue;
        }

        if (fprintf(tempFile, "%d|%s|%s|%.2f|%d|%s\n",
                    p.productId,
                    p.productName,
                    p.category,
                    p.price,
                    p.stockQuantity,
                    p.description) < 0)
        {
            printf("Error writing temporary file!\n");
            fclose(file);
            fclose(tempFile);
            remove("data/temp.txt");
            return;
        }
    }

    fclose(file);

    if (fclose(tempFile) != 0)
    {
        printf("Error closing temporary file!\n");
        remove("data/temp.txt");
        return;
    }

    if (!found)
    {
        remove("data/temp.txt");
        printf("Product ID %d not found.\n", searchId);
        return;
    }

    if (remove(PRODUCTS_FILE) != 0)
    {
        printf("Error replacing products file!\n");
        remove("data/temp.txt");
        return;
    }

    if (rename("data/temp.txt", PRODUCTS_FILE) != 0)
    {
        printf("Error saving updated products!\n");
        printf("Your temporary file is data/temp.txt\n");
        return;
    }

    printf("\nProduct deleted successfully!\n");
}

int checkStockAvailability(int productId, int requiredQty)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");

    if (file == NULL)
    {
        return 0;
    }

    if (requiredQty <= 0)
    {
        fclose(file);
        return 0;
    }

    Product p;
    char line[512];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId,
                   p.productName,
                   p.category,
                   &p.price,
                   &p.stockQuantity,
                   p.description) != 6)
        {
            continue;
        }

        if (p.productId == productId)
        {
            int available = (p.stockQuantity >= requiredQty);

            fclose(file);
            return available;
        }
    }

    fclose(file);

   return 0;
}
int updateStockAfterPurchase(int productId, int purchasedQty)
{
    if (purchasedQty <= 0)
        return 0;

    FILE *file = fopen("data/products.txt", "r");
    if (file == NULL)
        file = fopen("products.txt", "r");
    if (file == NULL)
        return 0;

    FILE *tempFile = fopen("data/temp.txt", "w");
    if (tempFile == NULL)
        tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL)
    {
        fclose(file);
        return 1;
    }

    char line[512];
    Product p;

    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &p.productId, p.productName, p.category, &p.price,
                   &p.stockQuantity, p.description) != 6)
        {
            fputs(line, tempFile);
            continue;
        }

        if (p.productId == productId)
        {
            p.stockQuantity -= purchasedQty;
            if (p.stockQuantity < 0)
                p.stockQuantity = 0;
        }
        fprintf(tempFile, "%d|%s|%s|%.2f|%d|%s\n",
                p.productId, p.productName, p.category, p.price, p.stockQuantity, p.description);
    }
    fclose(file);
    fclose(tempFile);
    remove("data/products.txt");
    remove("products.txt");
    if (rename("data/temp.txt", "data/products.txt") != 0)
        rename("temp.txt", "products.txt");


    return 1;
}