#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "models.h"
#include "products.h"
#include "ui.h"
#include "cart.h"
#include "orders.h"


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

int getNextProductId(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    int highestProductId = 0;
    char line[512];

    if (file == NULL)
    {
        if (errno == ENOENT)
        {
            return 1;
        }

        printf("Error opening products file while generating Product ID!\n");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        Product product;

        if (sscanf(line, "%d|%49[^|]|%29[^|]|%f|%d|%99[^\n]",
                   &product.productId,
                   product.productName,
                   product.category,
                   &product.price,
                   &product.stockQuantity,
                   product.description) == 6 &&
            product.productId > highestProductId)
        {
            highestProductId = product.productId;
        }
    }

    if (ferror(file))
    {
        printf("Error reading products file while generating Product ID!\n");
        fclose(file);
        return -1;
    }

    fclose(file);

    if (highestProductId == INT_MAX)
    {
        printf("Unable to generate Product ID: maximum ID reached.\n");
        return -1;
    }

    return highestProductId + 1;
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

    p.productId = getNextProductId();
    if (p.productId < 1)
    {
        fclose(file);
        return;
    }
    printf("Generated Product ID: %d\n", p.productId);

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

/* =====================================================================
 * Paginated product browsing + search by name
 * ===================================================================== */

#define PRODUCTS_PER_PAGE 20

/*
 * Parse one line of products.txt:  id|name|category|price|stock|description
 * Unlike sscanf("%99[^\n]") this accepts an empty description and CRLF
 * line endings. Returns 1 on success, 0 if the line is malformed.
 */
static int parseProductLine(const char *line, Product *p)
{
    char buf[512];
    char *fields[6];
    char *cursor;
    char *end;
    int i;

    snprintf(buf, sizeof(buf), "%s", line);
    buf[strcspn(buf, "\r\n")] = '\0';

    cursor = buf;
    for (i = 0; i < 5; i++)
    {
        char *bar = strchr(cursor, '|');

        if (bar == NULL)
            return 0;

        *bar = '\0';
        fields[i] = cursor;
        cursor = bar + 1;
    }
    fields[5] = cursor;

    p->productId = (int)strtol(fields[0], &end, 10);
    if (end == fields[0] || *end != '\0' || p->productId < 1)
        return 0;

    if (fields[1][0] == '\0')
        return 0;

    p->price = strtof(fields[3], &end);
    if (end == fields[3] || *end != '\0')
        return 0;

    p->stockQuantity = (int)strtol(fields[4], &end, 10);
    if (end == fields[4] || *end != '\0')
        return 0;

    snprintf(p->productName, sizeof(p->productName), "%s", fields[1]);
    snprintf(p->category, sizeof(p->category), "%s", fields[2]);
    snprintf(p->description, sizeof(p->description), "%s", fields[5]);

    return 1;
}

/* Case-insensitive "does text contain pattern" (portable strcasestr). */
static int containsIgnoreCase(const char *text, const char *pattern)
{
    size_t n = strlen(pattern);

    if (n == 0)
        return 1;

    for (; *text != '\0'; text++)
    {
        size_t i = 0;

        while (i < n && text[i] != '\0' &&
               tolower((unsigned char)text[i]) ==
                   tolower((unsigned char)pattern[i]))
        {
            i++;
        }

        if (i == n)
            return 1;
    }

    return 0;
}

static void trimInput(char *s)
{
    char *start = s;
    size_t len;

    while (*start == ' ' || *start == '\t')
        start++;

    if (start != s)
        memmove(s, start, strlen(start) + 1);

    len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                       s[len - 1] == '\r' || s[len - 1] == '\n'))
    {
        s[--len] = '\0';
    }
}

/* Reads one line from stdin, discards any overflow, trims it.
   Returns 0 on EOF/error. */
static int readLine(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;

    if (strchr(buf, '\n') == NULL)
    {
        int c;

        while ((c = getchar()) != '\n' && c != EOF)
        {
        }
    }

    trimInput(buf);
    return 1;
}

static int isAllDigits(const char *s)
{
    if (*s == '\0')
        return 0;

    for (; *s != '\0'; s++)
    {
        if (*s < '0' || *s > '9')
            return 0;
    }

    return 1;
}

/* Counts products whose name contains keyword (all products if keyword is
   empty). Returns -1 if the products file cannot be opened. */
static int countProducts(const char *keyword)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[512];
    Product p;
    int count = 0;

    if (file == NULL)
        return -1;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parseProductLine(line, &p) &&
            containsIgnoreCase(p.productName, keyword))
        {
            count++;
        }
    }

    fclose(file);
    return count;
}

/* Prints one page (PRODUCTS_PER_PAGE rows) of the matching products. */
static void printProductPage(const char *keyword, int page)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[512];
    Product p;
    int index = 0;
    int firstIndex = (page - 1) * PRODUCTS_PER_PAGE;
    int lastIndex = page * PRODUCTS_PER_PAGE;

    printf("\n%-8s %-36s %-12s %12s %7s\n",
           "ID", "Name", "Category", "Price (Rs.)", "Stock");
    printf("------------------------------------------------------------------------------\n");

    if (file == NULL)
        return;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (!parseProductLine(line, &p) ||
            !containsIgnoreCase(p.productName, keyword))
        {
            continue;
        }

        if (index >= firstIndex && index < lastIndex)
        {
            printf("%-8d %-36.36s %-12.12s %12.2f %7d\n",
                   p.productId, p.productName, p.category,
                   p.price, p.stockQuantity);
        }

        index++;

        if (index >= lastIndex)
            break;
    }

    printf("------------------------------------------------------------------------------\n");
    fclose(file);
}

/*
 * Interactive product browser: 20 products per page, next/previous,
 * jump to page, and search by product name (partial, case-insensitive).
 * initialKeyword may be NULL (show everything).
 */
static void browseProducts(const char *initialKeyword)
{
    char keyword[50] = "";
    char input[100];
    char message[100] = "";
    int page = 1;

    if (initialKeyword != NULL)
        snprintf(keyword, sizeof(keyword), "%s", initialKeyword);

    for (;;)
    {
        int total = countProducts(keyword);
        int pages;

        if (total < 0)
        {
            printf("No products found! Add products first.\n");
            return;
        }

        pages = (total + PRODUCTS_PER_PAGE - 1) / PRODUCTS_PER_PAGE;
        if (pages < 1)
            pages = 1;
        if (page > pages)
            page = pages;
        if (page < 1)
            page = 1;

        clearScreen();
        drawHeader(keyword[0] != '\0' ? "PRODUCT SEARCH" : "ALL PRODUCTS");

        if (keyword[0] != '\0')
            printf("\n  Search: \"%s\"  -  %d match(es)\n", keyword, total);
        else
            printf("\n  %d product(s) in total\n", total);

        printProductPage(keyword, page);

        if (total == 0)
        {
            printf("  No products found.\n");
        }
        else
        {
            int first = (page - 1) * PRODUCTS_PER_PAGE + 1;
            int last = page * PRODUCTS_PER_PAGE;

            if (last > total)
                last = total;

            printf("  Showing %d-%d of %d   |   Page %d of %d\n",
                   first, last, total, page, pages);
        }

        if (message[0] != '\0')
        {
            printf("\n  %s%s%s\n", YELLOW, message, RESET);
            message[0] = '\0';
        }

        printf("\n  [N] Next   [P] Previous   [number] Go to page\n");
        printf("  [S] Search by name");
        if (keyword[0] != '\0')
            printf("   [C] Show all products");
        printf("   [0] Back\n");
        printf("\n  Your choice (Enter = next page): ");

        if (!readLine(input, sizeof(input)))
            return;

        if (input[0] == '\0' || strcmp(input, "n") == 0 || strcmp(input, "N") == 0)
        {
            if (page < pages)
                page++;
            else
                snprintf(message, sizeof(message), "You are already on the last page.");
        }
        else if (strcmp(input, "p") == 0 || strcmp(input, "P") == 0)
        {
            if (page > 1)
                page--;
            else
                snprintf(message, sizeof(message), "You are already on the first page.");
        }
        else if (strcmp(input, "0") == 0 ||
                 strcmp(input, "q") == 0 || strcmp(input, "Q") == 0)
        {
            return;
        }
        else if (strcmp(input, "s") == 0 || strcmp(input, "S") == 0)
        {
            char name[50];

            printf("  Enter product name (or part of it): ");

            if (!readLine(name, sizeof(name)))
                return;

            if (name[0] == '\0')
            {
                snprintf(message, sizeof(message), "Search cancelled - nothing entered.");
            }
            else if (strchr(name, '|') != NULL)
            {
                snprintf(message, sizeof(message), "The | character is not allowed.");
            }
            else
            {
                snprintf(keyword, sizeof(keyword), "%s", name);
                page = 1;
            }
        }
        else if ((strcmp(input, "c") == 0 || strcmp(input, "C") == 0) &&
                 keyword[0] != '\0')
        {
            keyword[0] = '\0';
            page = 1;
        }
        else if (isAllDigits(input) && strlen(input) <= 9)
        {
            int target = atoi(input);

            if (target >= 1 && target <= pages)
                page = target;
            else
                snprintf(message, sizeof(message),
                         "Page %d does not exist (1-%d).", target, pages);
        }
        else
        {
            snprintf(message, sizeof(message), "Invalid choice. Try again.");
        }
    }
}

/* Shows every product (20 per page) - used by guests and customers. */
void viewProducts(void)
{
    browseProducts(NULL);
}

// Check product by ID or Name
int productMatches(const Product *p, const char *keyword)
{
    int isNumber = 1;

    for (int i = 0; keyword[i] != '\0'; i++)
    {
        if (keyword[i] < '0' || keyword[i] > '9')
        {
            isNumber = 0;
            break;
        }
    }

    if (isNumber)
    {
        return p->productId == atoi(keyword);
    }

    char lowerName[50];
    char lowerKeyword[50];

    strcpy(lowerName, p->productName);
    strcpy(lowerKeyword, keyword);

    for (int i = 0; lowerName[i] != '\0'; i++)
    {
        lowerName[i] = tolower((unsigned char)lowerName[i]);
    }

    for (int i = 0; lowerKeyword[i] != '\0'; i++)
    {
        lowerKeyword[i] = tolower((unsigned char)lowerKeyword[i]);
    }

    return strcmp(lowerName, lowerKeyword) == 0;
}

void viewProductsAndAddToCart(const char *username)
{
  
    browseProducts(NULL);

    char keyword[50];
    Product p;
    int found = 0;

    printf("\n  --------------------------------------------------\n");
    printf("  Now enter the product you want to buy.\n");
    printf("  You can type ID (e.g. 25) or exact name\n");
    printf("  --------------------------------------------------\n");

    while (found == 0)
    {
        printf("\n  Enter Product ID or Name to add to cart (0 = back): ");

        if (fgets(keyword, sizeof(keyword), stdin) == NULL) return;
        keyword[strcspn(keyword, "\r\n")] = '\0';
        trimInput(keyword);

        if (keyword[0] == '\0' || strcmp(keyword, "0") == 0) return;

        found = 0;
        FILE *file = fopen(PRODUCTS_FILE, "r");
        if (file == NULL) {
            printf("No products available.\n");
            return;
        }
        char line[512];
        Product temp;
        while (fgets(line, sizeof(line), file) != NULL)
        {
            if (parseProductLine(line, &temp)) {
                if (productMatches(&temp, keyword)) {
                    p = temp;
                    found = 1;
                    break;
                }
            }
        }
        fclose(file);

        if (found == 0) {
            printf("  Product '%s' not found! Check ID from list above or try partial name search.\n", keyword);
            printf("  Tip: Press S in the product list to search by name.\n");
        }
    }

    printf("\n--- Product Found! ---\n");
    printf("ID          : %d\n", p.productId);
    printf("Name        : %s\n", p.productName);
    printf("Category    : %s\n", p.category);
    printf("Price       : %.2f\n", p.price);
    printf("Stock       : %d\n", p.stockQuantity);
    printf("Description : %s\n", p.description);

    int action;
    printf("\n1. Add to Cart\n");
    printf("2. Go to Checkout\n");
    printf("0. Back to Products\n");
    printf("Enter your choice: ");

    if (scanf("%d", &action) != 1) {
        while (getchar() != '\n' && !feof(stdin));
        return;
    }
    while (getchar() != '\n' && !feof(stdin));

    if (action == 1 || action == 2) {
        int quantity;
        printf("Enter Quantity (Available: %d): ", p.stockQuantity);
        if (scanf("%d", &quantity) != 1 || quantity <= 0) {
            printf("Invalid quantity!\n");
            while (getchar() != '\n' && !feof(stdin));
            return;
        }
        while (getchar() != '\n' && !feof(stdin));

        if (quantity > p.stockQuantity) {
            printf("Insufficient stock! Only %d available.\n", p.stockQuantity);
            return;
        }

        if (!checkStockAvailability(p.productId, quantity)) {
            printf("Insufficient stock for Product ID %d.\n", p.productId);
            return;
        }

        if (action == 1) {
            addToCart(username, p.productId, quantity);
            viewCart(username);
        } else {
            startDirectCheckout(username, p.productId, quantity, p.price);
        }
    }
}
/* Prints the full details of one product, looked up by ID. */
static void showProductById(int searchId)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[512];
    Product p;
    int found = 0;

    if (file == NULL) {
        printf("No products available. File '%s' not found!\n", PRODUCTS_FILE);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (parseProductLine(line, &p) && p.productId == searchId) {
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
    fclose(file);

    if (!found) {
        printf("\n  Product ID %d not found.\n", searchId);
        // suggestion: ළඟම තියෙන IDs 5ක් පෙන්නනවා
        file = fopen(PRODUCTS_FILE, "r");
        if (file) {
            printf("\n  Did you mean one of these? (Closest IDs)\n");
            int count = 0;
            while (fgets(line, sizeof(line), file) != NULL && count < 5) {
                if (parseProductLine(line, &p)) {
                    if (abs(p.productId - searchId) <= 10) {
                        printf("    [%d] %s\n", p.productId, p.productName);
                        count++;
                    }
                }
            }
            fclose(file);
        }
        printf("\n  Tip: Search by name - e.g. type 'headphone' instead of ID\n");
    }
}
/* Search by Product ID (exact) or by name (partial, paginated results). */
void searchProduct(void)
{
    
    FILE *f = fopen(PRODUCTS_FILE, "r");
    if (!f) {
        printf("No products found! Add products first.\n");
        return;
    }
    char line[512];
    Product p;
    int total = 0, minId = INT_MAX, maxId = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (parseProductLine(line, &p)) {
            total++;
            if (p.productId < minId) minId = p.productId;
            if (p.productId > maxId) maxId = p.productId;
        }
    }
    fclose(f);

    if (total == 0) {
        printf("No products available.\n");
        return;
    }

    clearScreen();
    drawHeader("SEARCH PRODUCT");
    printf("\n  Total Products: %d\n", total);
    printf("  Available ID Range: %d to %d\n", minId, maxId);
    printf("  Tip: You can search by ID OR by name (partial name also works)\n");

    
    printf("\n  Quick Preview (first 10 products):\n");
    printf("  ---------------------------------------------\n");
    f = fopen(PRODUCTS_FILE, "r");
    int shown = 0;
    while (fgets(line, sizeof(line), f) != NULL && shown < 10) {
        if (parseProductLine(line, &p)) {
            printf("    [%d] %s - Rs. %.2f\n", p.productId, p.productName, p.price);
            shown++;
        }
    }
    fclose(f);
    printf("  ---------------------------------------------\n");

    char input[100];
    printf("\n  Enter Product ID or Name to search (0 = back): ");
    if (!readLine(input, sizeof(input))) return;
    if (input[0] == '\0' || strcmp(input, "0") == 0) return;
    if (strchr(input, '|') != NULL) {
        printf("The | character is not allowed.\n");
        return;
    }

    if (isAllDigits(input) && strlen(input) <= 9) {
        
        showProductById(atoi(input));
    } else {
       
        browseProducts(input);
    }
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