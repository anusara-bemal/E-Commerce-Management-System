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

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MAKE_DIR(p) mkdir((p), 0777)
#endif

#define PRODUCTS_PER_PAGE 20
#define LINE_SIZE 512
#define TEMP_FILE "data/temp.txt"
#define LINE_RULE "------------------------------------------------------------------------------\n"

typedef enum { OP_UPDATE, OP_DELETE, OP_REDUCE_STOCK } ProductOp;

/* ------------------------------------------------------------------ */
/*  Small helpers                                                      */
/* ------------------------------------------------------------------ */

static void ensureDataDir(void)
{
    MAKE_DIR("data"); /* fails harmlessly if it already exists */
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
        s[--len] = '\0';
}

/* Reads one line, discards overflow, trims it. Returns 0 on EOF/error. */
static int readLine(char *buf, size_t size)
{
    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;

    if (strchr(buf, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }
    trimInput(buf);
    return 1;
}

static int isAllDigits(const char *s)
{
    if (*s == '\0')
        return 0;
    for (; *s != '\0'; s++)
        if (*s < '0' || *s > '9')
            return 0;
    return 1;
}

static int parseInt(const char *s, int *out)
{
    char *end;
    long v;

    errno = 0;
    v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || errno == ERANGE || v > INT_MAX || v < INT_MIN)
        return 0;
    *out = (int)v;
    return 1;
}

static int isKey(const char *s, char key)
{
    return s[1] == '\0' && tolower((unsigned char)s[0]) == key;
}

static int containsIgnoreCase(const char *text, const char *pattern)
{
    size_t n = strlen(pattern);

    if (n == 0)
        return 1;

    for (; *text != '\0'; text++)
    {
        size_t i = 0;
        while (i < n && text[i] != '\0' &&
               tolower((unsigned char)text[i]) == tolower((unsigned char)pattern[i]))
            i++;
        if (i == n)
            return 1;
    }
    return 0;
}

static int equalsIgnoreCase(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
    return *a == *b;
}

/* ------------------------------------------------------------------ */
/*  File format:  id|name|category|price|stock|description             */
/* ------------------------------------------------------------------ */

/* Accepts empty category/description and CRLF. Returns 1 if valid. */
static int parseProductLine(const char *line, Product *p)
{
    char buf[LINE_SIZE];
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

static int writeProduct(FILE *f, const Product *p)
{
    return fprintf(f, "%d|%s|%s|%.2f|%d|%s\n", p->productId, p->productName,
                   p->category, p->price, p->stockQuantity, p->description);
}

static void printProduct(const Product *p)
{
    printf("\n--- Product Found! ---\n");
    printf("ID          : %d\n", p->productId);
    printf("Name        : %s\n", p->productName);
    printf("Category    : %s\n", p->category);
    printf("Price       : %.2f\n", p->price);
    printf("Stock       : %d\n", p->stockQuantity);
    printf("Description : %s\n", p->description);
}

/* 1 = found, 0 = not found, -1 = cannot open file */
static int findProductById(int id, Product *out)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[LINE_SIZE];
    int found = 0;

    if (file == NULL)
        return -1;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parseProductLine(line, out) && out->productId == id)
        {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

/* Product ID (digits) or exact name (case-insensitive). */
int productMatches(const Product *p, const char *keyword)
{
    if (isAllDigits(keyword))
        return strlen(keyword) <= 9 && p->productId == atoi(keyword);
    return equalsIgnoreCase(p->productName, keyword);
}

/* 1 = found, 0 = not found, -1 = cannot open file */
static int findProduct(const char *keyword, Product *out)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[LINE_SIZE];
    int found = 0;

    if (file == NULL)
        return -1;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parseProductLine(line, out) && productMatches(out, keyword))
        {
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

/* Windows rename() fails if target exists, so remove first. */
static int replaceFile(const char *tmp, const char *target)
{
    if (remove(target) != 0 && errno != ENOENT)
        return 0;
    return rename(tmp, target) == 0;
}

/* ------------------------------------------------------------------ */
/*  Edit prompts                                                       */
/* ------------------------------------------------------------------ */

static void editText(const char *label, char *field, size_t size)
{
    char input[100];

    printf("%s [%s]: ", label, field);
    if (!readLine(input, sizeof(input)) || input[0] == '\0')
        return;

    if (strchr(input, '|') != NULL)
        printf("The | character is not allowed. Keeping old %s.\n", label);
    else
        snprintf(field, size, "%s", input);
}

static void editProduct(Product *p)
{
    char input[100];
    char *end;
    int v;

    printf("\nProduct found! Leave a field empty to keep its old value.\n");

    editText("Name", p->productName, sizeof(p->productName));
    editText("Category", p->category, sizeof(p->category));

    printf("Price [%.2f]: ", p->price);
    if (readLine(input, sizeof(input)) && input[0] != '\0')
    {
        float price = strtof(input, &end);
        if (end != input && *end == '\0' && price >= 0)
            p->price = price;
        else
            printf("Invalid price. Keeping old price.\n");
    }

    printf("Stock [%d]: ", p->stockQuantity);
    if (readLine(input, sizeof(input)) && input[0] != '\0')
    {
        if (parseInt(input, &v) && v >= 0)
            p->stockQuantity = v;
        else
            printf("Invalid stock quantity. Keeping old stock.\n");
    }

    editText("Description", p->description, sizeof(p->description));
}

/*
 * One routine for update / delete / stock reduction.
 * Returns 1 = done, -1 = product not found, 0 = file error.
 * Unparseable lines are copied through untouched (no data loss).
 */
static int rewriteProducts(ProductOp op, int id, int qty)
{
    FILE *in = fopen(PRODUCTS_FILE, "r");
    FILE *out;
    char line[LINE_SIZE];
    Product p;
    int found = 0, ok = 1;

    if (in == NULL)
        return 0;

    out = fopen(TEMP_FILE, "w");
    if (out == NULL)
    {
        fclose(in);
        return 0;
    }

    while (fgets(line, sizeof(line), in) != NULL)
    {
        if (!parseProductLine(line, &p))
        {
            fputs(line, out);
            continue;
        }

        if (p.productId == id)
        {
            found = 1;
            if (op == OP_DELETE)
                continue;
            if (op == OP_UPDATE)
                editProduct(&p);
            else
            {
                p.stockQuantity -= qty;
                if (p.stockQuantity < 0)
                    p.stockQuantity = 0;
            }
        }

        if (writeProduct(out, &p) < 0)
        {
            ok = 0;
            break;
        }
    }

    fclose(in);
    if (fclose(out) != 0)
        ok = 0;

    if (!ok || !found)
    {
        remove(TEMP_FILE);
        return ok ? -1 : 0;
    }

    return replaceFile(TEMP_FILE, PRODUCTS_FILE) ? 1 : 0;
}

static int askProductId(const char *prompt, int *id)
{
    char input[32];

    printf("%s", prompt);
    if (!readLine(input, sizeof(input)) || !parseInt(input, id))
    {
        printf("Invalid Product ID!\n");
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Menu                                                               */
/* ------------------------------------------------------------------ */

void productMenu(void)
{
    char input[16];
    int choice;

    ensureDataDir();

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

        if (!readLine(input, sizeof(input)))
            return;
        if (!parseInt(input, &choice))
            choice = -1;

        switch (choice)
        {
        case 1: addProduct();    break;
        case 2: viewProducts();  break;
        case 3: searchProduct(); break;
        case 4: updateProduct(); break;
        case 5: deleteProduct(); break;
        case 6: printf("Returning...\n"); break;
        default: printf("Invalid choice!\n");
        }

        if (choice != 6)
        {
            printf("\nPress Enter...");
            getchar();
        }
    } while (choice != 6);
}

/* ------------------------------------------------------------------ */
/*  Add                                                                */
/* ------------------------------------------------------------------ */

int getNextProductId(void)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[LINE_SIZE];
    Product p;
    int highest = 0;

    if (file == NULL)
    {
        if (errno == ENOENT)
            return 1;
        printf("Error opening products file while generating Product ID!\n");
        return -1;
    }

    while (fgets(line, sizeof(line), file) != NULL)
        if (parseProductLine(line, &p) && p.productId > highest)
            highest = p.productId;

    if (ferror(file))
    {
        printf("Error reading products file while generating Product ID!\n");
        fclose(file);
        return -1;
    }
    fclose(file);

    if (highest == INT_MAX)
    {
        printf("Unable to generate Product ID: maximum ID reached.\n");
        return -1;
    }
    return highest + 1;
}

void addProduct(void)
{
    Product p;
    char buf[50];
    char *end;
    FILE *file;

    clearScreen();
    drawHeader("ADD PRODUCT");
    ensureDataDir();

    p.productId = getNextProductId();
    if (p.productId < 1)
    {
        showError("Cannot generate ID! Check data/products.txt");
        pressEnterToContinue();
        return;
    }
    printf("\n  New ID: %d (auto)\n", p.productId);

    printf("  Name: ");
    if (!readLine(p.productName, sizeof(p.productName)) || p.productName[0] == '\0')
    {
        showError("Name cannot be empty!");
        pressEnterToContinue();
        return;
    }

    printf("  Category: ");
    if (!readLine(p.category, sizeof(p.category)))
        p.category[0] = '\0';

    printf("  Price: ");
    if (!readLine(buf, sizeof(buf)))
        return;
    p.price = strtof(buf, &end);
    if (end == buf || *end != '\0' || p.price < 0)
    {
        showError("Invalid or negative price!");
        pressEnterToContinue();
        return;
    }

    printf("  Stock: ");
    if (!readLine(buf, sizeof(buf)))
        return;
    if (!parseInt(buf, &p.stockQuantity) || p.stockQuantity < 0)
    {
        showError("Invalid or negative stock!");
        pressEnterToContinue();
        return;
    }

    printf("  Description: ");
    if (!readLine(p.description, sizeof(p.description)))
        p.description[0] = '\0';

    if (strchr(p.productName, '|') || strchr(p.category, '|') ||
        strchr(p.description, '|'))
    {
        showError("Character '|' not allowed!");
        pressEnterToContinue();
        return;
    }

    file = fopen(PRODUCTS_FILE, "a");
    if (file == NULL)
    {
        showError("Cannot open products file!");
        printf(DIM "  Tried: %s\n" RESET, PRODUCTS_FILE);
        printf(DIM "  Error: %s\n" RESET, strerror(errno));
        pressEnterToContinue();
        return;
    }

    writeProduct(file, &p);
    fclose(file);
    showSuccess("Product added!");
    pressEnterToContinue();
}

/* ------------------------------------------------------------------ */
/*  Browse (paginated + name search)                                   */
/* ------------------------------------------------------------------ */

/* Number of products whose name contains keyword; -1 if file can't open. */
static int countProducts(const char *keyword)
{
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[LINE_SIZE];
    Product p;
    int count = 0;

    if (file == NULL)
        return -1;

    while (fgets(line, sizeof(line), file) != NULL)
        if (parseProductLine(line, &p) && containsIgnoreCase(p.productName, keyword))
            count++;

    fclose(file);
    return count;
}
static void printProductPage(const char *keyword, int page){
    FILE *file = fopen(PRODUCTS_FILE, "r");
    char line[512]; Product p; int index = 0;
    int firstIndex = (page - 1) * PRODUCTS_PER_PAGE;
    int lastIndex = page * PRODUCTS_PER_PAGE;
    
    // Fixed widths - මේවා වෙනස් කරන්න එපා නැත්තන් border කැඩෙනවා
    // ID:5 | Name:30 | Category:12 | Price:12 | Stock:7
    printf("\n");
    printf("  " MAGENTA "╭─────┬──────────────────────────────┬──────────────┬──────────────┬────────╮\n" RESET);
    printf("  " MAGENTA "│" RESET BOLD WHITE " %-3s " RESET MAGENTA "│" RESET BOLD WHITE " %-28s " RESET MAGENTA "│" RESET BOLD WHITE " %-12s " RESET MAGENTA "│" RESET BOLD WHITE " %-12s " RESET MAGENTA "│" RESET BOLD WHITE " %-6s " RESET MAGENTA "│\n" RESET, "ID", "Name", "Category", "Price(Rs.)", "Stock");
    printf("  " MAGENTA "├─────┼──────────────────────────────┼──────────────┼──────────────┼────────┤\n" RESET);

    if (file == NULL) {
        printf("  " MAGENTA "│" RESET " No file found...                                                     " RESET MAGENTA "│\n" RESET);
        printf("  " MAGENTA "╰─────┴──────────────────────────────┴──────────────┴──────────────┴────────╯\n" RESET);
        return;
    }

    int shown = 0;
    while (fgets(line, sizeof(line), file) != NULL){
        if (!parseProductLine(line, &p) || !containsIgnoreCase(p.productName, keyword)) continue;
        if (index >= firstIndex && index < lastIndex){
            
            const char* sColor = GREEN;
            if (p.stockQuantity == 0) sColor = RED;
            else if (p.stockQuantity < 20) sColor = YELLOW;

            // සැම row එකක්ම එකම length එක - තමයි වැදගත්ම දේ
            printf("  " MAGENTA "│" RESET " %3d " MAGENTA "│" RESET " %-28.28s " MAGENTA "│" RESET " %-12.12s " MAGENTA "│" RESET " %10.2f   " MAGENTA "│" RESET " %s● %3d%s " MAGENTA " │\n" RESET,
                   p.productId, 
                   p.productName, 
                   p.category, 
                   p.price,
                   sColor, p.stockQuantity, RESET);
            shown++;
        }
        index++;
        if (index >= lastIndex) break;
    }

    if (shown == 0) {
        printf("  " MAGENTA "│" RESET " No products to display...                                             " MAGENTA "│\n" RESET);
    }

    printf("  " MAGENTA "╰─────┴──────────────────────────────┴──────────────┴──────────────┴────────╯\n" RESET);
    fclose(file);
}
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
            clearScreen();
            drawHeader("ERROR");
            printf("\n  " RED "✘ Cannot open '%s': %s\n" RESET, PRODUCTS_FILE, strerror(errno));
            showWarning("Run from project root: /mnt/c/.../E-Commerce-Management-System");
            pressEnterToContinue();
            return;
        }

        pages = (total + PRODUCTS_PER_PAGE - 1) / PRODUCTS_PER_PAGE;
        if (pages < 1) pages = 1;
        if (page > pages) page = pages;
        if (page < 1) page = 1;

        clearScreen();
        drawHeader(keyword[0] != '\0' ? "PRODUCT SEARCH" : "PRODUCT CATALOG");

        if (keyword[0] != '\0')
            printf("\n  " CYAN "🔍 Search:" RESET " \"%s\"  " DIM "→ %d match(es)\n" RESET, keyword, total);
        else
            printf("\n  " WHITE BOLD "📦 Total Products: %d\n" RESET, total);

        printProductPage(keyword, page);

        if (total == 0)
        {
            printf("\n  " YELLOW "  No products found. Try different keyword.\n" RESET);
        }
        else
        {
            int first = (page - 1) * PRODUCTS_PER_PAGE + 1;
            int last = page * PRODUCTS_PER_PAGE;
            if (last > total) last = total;
            printf("\n  " DIM "Showing %d-%d of %d   │   Page %d of %d\n" RESET, first, last, total, page, pages);
            
            // Progress bar
            printf("  ");
            int barWidth = 40;
            int filled = (pages > 0) ? (page * barWidth / pages) : 0;
            printf(MAGENTA "[");
            for(int i=0;i<barWidth;i++){
                if(i < filled) printf("█");
                else printf("░");
            }
            printf("] %d%%\n" RESET, (pages>0)? (page*100/pages) : 0);
        }

        if (message[0] != '\0')
        {
            printf("\n  " YELLOW "⚠ %s\n" RESET, message);
            message[0] = '\0';
        }

        // Styled controls
        printf("\n");
        printf("  " DIM "────────────────────────────────────────────────────────\n" RESET);
        printf("  " GREEN BOLD "[N]" RESET " Next  " GREEN BOLD "[P]" RESET " Prev  " CYAN BOLD "[#]" RESET " Go page  " CYAN BOLD "[S]" RESET " Search\n");
        printf("  ");
        if (keyword[0] != '\0') printf("" YELLOW BOLD "[C]" RESET " Clear filter  ");
        printf("" RED BOLD "[0]" RESET " Back to menu");
        if(total>0) printf("  " DIM "(Enter = Next)\n" RESET);
        else printf("\n");
        printf("  " DIM "────────────────────────────────────────────────────────\n" RESET);
        printf("\n  " CYAN "❯" RESET " Your choice: ");

        if (!readLine(input, sizeof(input)))
            return;

        if (input[0] == '\0' || isKey(input, 'n') || isKey(input, 'N'))
        {
            if (page < pages) page++;
            else snprintf(message, sizeof(message), "Already on last page!");
        }
        else if (isKey(input, 'p') || isKey(input, 'P'))
        {
            if (page > 1) page--;
            else snprintf(message, sizeof(message), "Already on first page!");
        }
        else if (strcmp(input, "0") == 0 || isKey(input, 'q') || isKey(input, 'Q'))
        {
            return;
        }
        else if (isKey(input, 's') || isKey(input, 'S'))
        {
            char name[50];
            printf("  " CYAN "Enter name to search: " RESET);
            if (!readLine(name, sizeof(name))) return;

            if (name[0] == '\0')
                snprintf(message, sizeof(message), "Search cancelled.");
            else if (strchr(name, '|') != NULL)
                snprintf(message, sizeof(message), "'|' not allowed.");
            else
            {
                snprintf(keyword, sizeof(keyword), "%s", name);
                page = 1;
            }
        }
        else if ((isKey(input, 'c') || isKey(input, 'C')) && keyword[0] != '\0')
        {
            keyword[0] = '\0';
            page = 1;
            snprintf(message, sizeof(message), "Filter cleared, showing all.");
        }
        else if (isAllDigits(input) && strlen(input) <= 9)
        {
            int target = atoi(input);
            if (target >= 1 && target <= pages)
                page = target;
            else
                snprintf(message, sizeof(message), "Page %d invalid (1-%d).", target, pages);
        }
        else
        {
            snprintf(message, sizeof(message), "Invalid choice '%.20s'", input);
        }
    }
}

void viewProducts(void)
{
    browseProducts(NULL);
}

/* ------------------------------------------------------------------ */
/*  Customer: pick a product and add to cart / checkout                */
/* ------------------------------------------------------------------ */

void viewProductsAndAddToCart(const char *username)
{
    char keyword[50];
    char input[32];
    Product p;
    int action, quantity;

    browseProducts(NULL);

    printf("\n  --------------------------------------------------\n");
    printf("  Now enter the product you want to buy.\n");
    printf("  You can type ID (e.g. 25) or exact name\n");
    printf("  --------------------------------------------------\n");

    for (;;)
    {
        int r;

        printf("\n  Enter Product ID or Name to add to cart (0 = back): ");
        if (!readLine(keyword, sizeof(keyword)) ||
            keyword[0] == '\0' || strcmp(keyword, "0") == 0)
            return;

        r = findProduct(keyword, &p);
        if (r < 0)
        {
            printf("No products available.\n");
            return;
        }
        if (r > 0)
            break;

        printf("  Product '%s' not found! Check ID from list above.\n", keyword);
        printf("  Tip: Press S in the product list to search by name.\n");
    }

    printProduct(&p);

    printf("\n1. Add to Cart\n");
    printf("2. Go to Checkout\n");
    printf("0. Back to Products\n");
    printf("Enter your choice: ");
    if (!readLine(input, sizeof(input)) || !parseInt(input, &action) ||
        (action != 1 && action != 2))
        return;

    printf("Enter Quantity (Available: %d): ", p.stockQuantity);
    if (!readLine(input, sizeof(input)) || !parseInt(input, &quantity) || quantity <= 0)
    {
        printf("Invalid quantity!\n");
        return;
    }

    if (!checkStockAvailability(p.productId, quantity))
    {
        printf("Insufficient stock! Only %d available.\n", p.stockQuantity);
        return;
    }

    if (action == 1)
    {
        addToCart(username, p.productId, quantity);
        viewCart(username);
    }
    else
    {
        startDirectCheckout(username, p.productId, quantity, p.price);
    }
}

/* ------------------------------------------------------------------ */
/*  Search (admin)                                                     */
/* ------------------------------------------------------------------ */

static void showProductById(int searchId)
{
    Product p;
    int r = findProductById(searchId, &p);

    if (r < 0)
    {
        printf("Cannot open '%s': %s\n", PRODUCTS_FILE, strerror(errno));
        return;
    }
    if (r > 0)
    {
        printProduct(&p);
        return;
    }

    printf("\n  Product ID %d not found.\n", searchId);

    FILE *file = fopen(PRODUCTS_FILE, "r");
    if (file != NULL)
    {
        char line[LINE_SIZE];
        int count = 0;

        printf("\n  Did you mean one of these? (Closest IDs)\n");
        while (count < 5 && fgets(line, sizeof(line), file) != NULL)
        {
            if (parseProductLine(line, &p) && abs(p.productId - searchId) <= 10)
            {
                printf("    [%d] %s\n", p.productId, p.productName);
                count++;
            }
        }
        fclose(file);
    }
    printf("\n  Tip: Search by name - e.g. type 'headphone' instead of ID\n");
}

void searchProduct(void)
{
    FILE *f = fopen(PRODUCTS_FILE, "r");
    char line[LINE_SIZE];
    char input[100];
    Product p;
    Product preview[10];
    int total = 0, minId = INT_MAX, maxId = 0, i;

    if (f == NULL)
    {
        clearScreen();
        drawHeader("SEARCH PRODUCT");
        printf("\n");
        char msg[120];
        snprintf(msg, sizeof(msg), "Cannot open '%s'", PRODUCTS_FILE);
        showError(msg);
        showInfo("Add products first.");
        pressEnterToContinue();
        return;
    }

    while (fgets(line, sizeof(line), f) != NULL)
    {
        if (!parseProductLine(line, &p))
            continue;
        if (total < 10)
            preview[total] = p;
        total++;
        if (p.productId < minId) minId = p.productId;
        if (p.productId > maxId) maxId = p.productId;
    }
    fclose(f);

    if (total == 0)
    {
        clearScreen();
        drawHeader("SEARCH PRODUCT");
        printf("\n");
        showInfo("No products available.");
        pressEnterToContinue();
        return;
    }

    clearScreen();
    drawHeader("SEARCH PRODUCT");

    printf("\n  ╭────────────────────────────────────────╮\n");
    printf("  │  Total Products: " BOLD GREEN "%-4d" RESET "                            \n", total);
    printf("  │  Available ID  : " CYAN "%d" RESET " to " CYAN "%d" RESET "                         \n", minId, maxId);
    printf("  │  " DIM "Tip: Search by ID or name (partial also works)" RESET " \n");
    printf("  ╰────────────────────────────────────────╯\n");

    printf("\n  " BOLD WHITE "Quick Preview (first 10)\n" RESET);
    printf("  ╭────────────────────────────────────────╮\n");
    for (i = 0; i < total && i < 10; i++)
    {
        printf("  │  " CYAN "[%d]" RESET " %-20.20s " GREEN "Rs. %.2f" RESET "      \n",
               preview[i].productId, preview[i].productName, preview[i].price);
    }
    printf("  ╰────────────────────────────────────────╯\n");

    printf("\n  " CYAN "❯" RESET " Enter Product ID or Name to search (" RED "0 = back" RESET "): ");
    if (!readLine(input, sizeof(input)) || input[0] == '\0' || strcmp(input, "0") == 0)
        return;

    if (strchr(input, '|') != NULL)
    {
        showError("The | character is not allowed.");
        pressEnterToContinue();
        return;
    }

    if (isAllDigits(input) && strlen(input) <= 9)
        showProductById(atoi(input));
    else
        browseProducts(input);
}/* ------------------------------------------------------------------ */
/*  Update / Delete / Stock                                            */
/* ------------------------------------------------------------------ */

static void reportRewriteResult(int r, int id, const char *successMsg)
{
    if (r > 0)
        printf("\n%s\n", successMsg);
    else if (r < 0)
        printf("Product ID %d not found.\n", id);
    else
        printf("Error accessing '%s': %s\n", PRODUCTS_FILE, strerror(errno));
}

void updateProduct(void)
{
    int id;

    if (!askProductId("\nEnter Product ID to update: ", &id))
        return;
    reportRewriteResult(rewriteProducts(OP_UPDATE, id, 0), id,
                        "Product updated successfully!");
}

void deleteProduct(void)
{
    int id;

    if (!askProductId("\nEnter Product ID to delete: ", &id))
        return;
    reportRewriteResult(rewriteProducts(OP_DELETE, id, 0), id,
                        "Product deleted successfully!");
}

int checkStockAvailability(int productId, int requiredQty)
{
    Product p;

    if (requiredQty <= 0)
        return 0;
    return findProductById(productId, &p) == 1 && p.stockQuantity >= requiredQty;
}

/* Returns 1 on success, 0 on failure (file error or unknown product). */
int updateStockAfterPurchase(int productId, int purchasedQty)
{
    if (purchasedQty <= 0)
        return 0;
    return rewriteProducts(OP_REDUCE_STOCK, productId, purchasedQty) == 1;
}