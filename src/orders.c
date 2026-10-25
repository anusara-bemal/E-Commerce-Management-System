#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "orders.h"
#include "cart.h"
#include "products.h"

#define ORDERS_FILE "data/orders.txt"
#define TEMP_ORDERS_FILE "data/temp_orders.txt"
#define MAX_LINE 300

// Valid order statuses
#define STATUS_PENDING 0
#define STATUS_PAID    1
#define STATUS_SHIPPED 2

// --------------------------------------------------
// Calculate total price
// cartItem contains quantities, not product IDs.
// --------------------------------------------------
void order_process(const int *cartItem, int quantity,
float *unitPrice, float *totalPrice)
{
if (cartItem == NULL || unitPrice == NULL || totalPrice == NULL ||
quantity <= 0)
{
if (totalPrice != NULL)
*totalPrice = 0.0f;
    return;
}

*totalPrice = 0.0f;

for (int i = 0; i < quantity; i++)
{
    if (cartItem[i] < 0 || unitPrice[i] < 0.0f)
    {
        *totalPrice = 0.0f;
        return;
    }

    *totalPrice += cartItem[i] * unitPrice[i];
}


}

// --------------------------------------------------
// Convert status number to text
// --------------------------------------------------
const char *statusToText(int status)
{
switch (status)
{
case STATUS_PENDING:
return "PENDING";


    case STATUS_PAID:
        return "PAID";

    case STATUS_SHIPPED:
        return "SHIPPED";

    default:
        return "UNKNOWN";
}


}

// --------------------------------------------------
// Generate next order ID
// Uses the largest existing ID + 1.
// --------------------------------------------------

static int generateOrderId(void)
{
    FILE *file = fopen(ORDERS_FILE, "r");

    if (file == NULL)
    {
        /* Create the first order only if the file does not exist. */
        file = fopen(ORDERS_FILE, "a");

        if (file == NULL)
        {
            return -1;
        }

        if (fclose(file) != 0)
        {
            return -1;
        }

        return 1;
    }

    char line[MAX_LINE];
    int id;
    int maxId = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (sscanf(line, "%d|", &id) == 1 && id > maxId)
        {
            maxId = id;
        }
    }

    if (ferror(file))
    {
        fclose(file);
        return -1;
    }

    fclose(file);

    if (maxId == 2147483647)
    {
        return -1;
    }

    return maxId + 1;
}


// --------------------------------------------------
// Place order
// --------------------------------------------------

int placeOrder(
    const char *username,
    const char *phoneNumber,
    const char *address,
    int itemCount,
    float totalAmount)
{
    if (username == NULL ||
        phoneNumber == NULL ||
        address == NULL ||
        username[0] == '\0' ||
        phoneNumber[0] == '\0' ||
        address[0] == '\0')
    {
        printf("\n[Error] Invalid customer details!\n");
        return 0;
    }

    if (strchr(username, '|') != NULL ||
        strchr(phoneNumber, '|') != NULL ||
        strchr(address, '|') != NULL ||
        strpbrk(username, "\r\n") != NULL ||
        strpbrk(phoneNumber, "\r\n") != NULL ||
        strpbrk(address, "\r\n") != NULL)
    {
        printf("\n[Error] Customer details contain invalid characters!\n");
        return 0;
    }

    if (itemCount <= 0 ||
        totalAmount <= 0.0f ||
        totalAmount != totalAmount ||
        totalAmount > 1.0e30f)
    {
        printf("\n[Error] Invalid order details!\n");
        return 0;
    }

    int newId = generateOrderId();

    if (newId < 0)
    {
        printf("\n[Error] Could not generate an order ID!\n");
        return 0;
    }

    FILE *file = fopen(ORDERS_FILE, "a");

    if (file == NULL)
    {
        printf("\n[Error] Could not open %s!\n", ORDERS_FILE);
        return 0;
    }

    int writeOk = fprintf(
        file,
        "%d|%s|%s|%s|%d|%.2f|%d\n",
        newId,
        username,
        phoneNumber,
        address,
        itemCount,
        totalAmount,
        STATUS_PENDING
    ) >= 0;

    if (fclose(file) != 0)
    {
        writeOk = 0;
    }

    if (!writeOk)
    {
        printf("\n[Error] Failed to save the order!\n");
        return 0;
    }

    printf("\n================================\n");
    printf("          ORDER PLACED!\n");
    printf("================================\n");
    printf("Order ID : %d\n", newId);
    printf("Customer : %s\n", username);
    printf("Items    : %d\n", itemCount);
    printf("Total    : Rs. %.2f\n", totalAmount);
    printf("Status   : %s\n", statusToText(STATUS_PENDING));
    printf("================================\n");

    return 1;
}


// --------------------------------------------------
// View orders belonging to a customer
// --------------------------------------------------
void myOrders(char username[])
{
if (username == NULL || username[0] == '\0')
{
printf("\n[Error] Invalid username!\n");
return;
}


FILE *file = fopen(ORDERS_FILE, "r");

if (file == NULL)
{
    printf("\nYou have no orders yet.\n");
    return;
}

printf("\n================ MY ORDERS ================\n");
printf("%-6s %-20s %-8s %-12s %-10s\n",
       "ID", "ADDRESS", "ITEMS", "TOTAL", "STATUS");

printf("-------------------------------------------------------------\n");

char line[MAX_LINE];
int found = 0;

while (fgets(line, sizeof(line), file) != NULL)
{
    int id;
    int itemCount;
    int status;
    float total;

    char name[30];
    char phone[15];
    char address[100];

    int fields = sscanf(line,
                        "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
                        &id, name, phone, address,
                        &itemCount, &total, &status);

    if (fields != 7)
        continue;

    if (strcmp(name, username) == 0)
    {
        printf("%-6d %-20.20s %-8d %-12.2f %-10s\n",
               id, address, itemCount, total,
               statusToText(status));

        found++;
    }
}

fclose(file);

if (found == 0)
    printf("You have no orders yet.\n");

printf("============================================\n");


}

// --------------------------------------------------
// Admin: view all orders
// --------------------------------------------------
void updateOrderStatus(void)
{
FILE *file = fopen(ORDERS_FILE, "r");


if (file == NULL)
{
    printf("\nNo orders found or orders file is unavailable.\n");
    return;
}

printf("\n================ ALL ORDERS ================\n");
printf("%-6s %-15s %-12s %-10s\n",
       "ID", "CUSTOMER", "TOTAL", "STATUS");

printf("---------------------------------------------\n");

char line[MAX_LINE];
int found = 0;

while (fgets(line, sizeof(line), file) != NULL)
{
    int id;
    int itemCount;
    int status;
    float total;

    char name[30];
    char phone[15];
    char address[100];

    int fields = sscanf(line,
                        "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
                        &id, name, phone, address,
                        &itemCount, &total, &status);

    if (fields != 7)
        continue;

    printf("%-6d %-15.15s %-12.2f %-10s\n",
           id, name, total, statusToText(status));

    found++;
}

fclose(file);

if (found == 0)
{
    printf("\nNo valid orders found.\n");
    return;
}

int orderId;
int newStatus;

printf("\nOrder ID (0 = cancel): ");

if (scanf("%d", &orderId) != 1)
{
    printf("\n[Error] Invalid input!\n");

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }

    return;
}

if (orderId == 0)
    return;

if (orderId < 1)
{
    printf("\n[Error] Invalid order ID!\n");
    return;
}

printf("\nNew Status:\n");
printf("0 = PENDING\n");
printf("1 = PAID\n");
printf("2 = SHIPPED\n");
printf("Enter status: ");

if (scanf("%d", &newStatus) != 1)
{
    printf("\n[Error] Invalid status input!\n");

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }

    return;
}

if (newStatus < STATUS_PENDING || newStatus > STATUS_SHIPPED)
{
    printf("\n[Error] Invalid status! Choose 0, 1 or 2.\n");
    return;
}

FILE *oldFile = fopen(ORDERS_FILE, "r");

if (oldFile == NULL)
{
    printf("\n[Error] Could not open orders file!\n");
    return;
}

FILE *tempFile = fopen(TEMP_ORDERS_FILE, "w");

if (tempFile == NULL)
{
    fclose(oldFile);
    printf("\n[Error] Could not create temporary orders file!\n");
    return;
}

int orderFound = 0;
int writeError = 0;

while (fgets(line, sizeof(line), oldFile) != NULL)
{
    int id;
    int itemCount;
    int status;
    float total;

    char name[30];
    char phone[15];
    char address[100];

    int fields = sscanf(line,
                        "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
                        &id, name, phone, address,
                        &itemCount, &total, &status);

    if (fields != 7)
    {
        // Preserve malformed records instead of silently deleting them.
        if (fputs(line, tempFile) == EOF)
            writeError = 1;

        continue;
    }

    if (id == orderId)
    {
        status = newStatus;
        orderFound = 1;
    }

    if (fprintf(tempFile, "%d|%s|%s|%s|%d|%.2f|%d\n",
                id, name, phone, address,
                itemCount, total, status) < 0)
    {
        writeError = 1;
        break;
    }
}

if (ferror(oldFile))
    writeError = 1;

if (fclose(oldFile) != 0)
    writeError = 1;

if (fclose(tempFile) != 0)
    writeError = 1;

if (writeError)
{
    remove(TEMP_ORDERS_FILE);
    printf("\n[Error] Failed to write updated orders!\n");
    return;
}

if (!orderFound)
{
    remove(TEMP_ORDERS_FILE);
    printf("\n[Error] Order ID %d was not found!\n", orderId);
    return;
}

// Replace the original file only after successful processing.
if (remove(ORDERS_FILE) != 0)
{
    printf("\n[Error] Could not replace the original orders file!\n");
    printf("Updated data remains in %s\n", TEMP_ORDERS_FILE);
    return;
}

if (rename(TEMP_ORDERS_FILE, ORDERS_FILE) != 0)
{
    printf("\n[Error] Could not rename the temporary file!\n");
    printf("Recovery file: %s\n", TEMP_ORDERS_FILE);
    return;
}

printf("\n[OK] Order %d status updated to %s!\n",
       orderId, statusToText(newStatus));


}

void checkout(const char *username)
{
    #define MAX_CHECKOUT_ITEMS 100

    char line[512];
    char phoneNumber[30];
    char address[200];

    int productIds[MAX_CHECKOUT_ITEMS];
    int quantities[MAX_CHECKOUT_ITEMS];
    float unitPrices[MAX_CHECKOUT_ITEMS];

    int itemCount = 0;
    float totalAmount = 0.0f;

    if (username == NULL || username[0] == '\0')
    {
        printf("Invalid customer account.\n");
        return;
    }

    FILE *cartFile = fopen("data/cart.txt", "r");

    if (cartFile == NULL)
    {
        printf("Unable to open cart file.\n");
        return;
    }

    /* Read this customer's cart */
    while (fgets(line, sizeof(line), cartFile) != NULL)
    {
        char cartUsername[50];
        char productName[50];

        int productId;
        int quantity;
        float unitPrice;
        float subtotal;

        if (sscanf(line, "%49[^|]|%d|%49[^|]|%d|%f|%f",
                   cartUsername, &productId, productName,
                   &quantity, &unitPrice, &subtotal) != 6)
        {
            continue;
        }

        if (strcmp(cartUsername, username) != 0)
            continue;

        if (productId <= 0 || quantity <= 0 ||
            unitPrice <= 0.0f)
        {
            fclose(cartFile);
            printf("Invalid item found in your cart.\n");
            return;
        }

        int index = -1;

        for (int i = 0; i < itemCount; i++)
        {
            if (productIds[i] == productId)
            {
                index = i;
                break;
            }
        }

        if (index >= 0)
        {
            if (quantities[index] > 2147483647 - quantity)
            {
                fclose(cartFile);
                printf("Cart quantity is too large.\n");
                return;
            }

            quantities[index] += quantity;
        }
        else
        {
            if (itemCount >= MAX_CHECKOUT_ITEMS)
            {
                fclose(cartFile);
                printf("Too many products in your cart.\n");
                return;
            }

            productIds[itemCount] = productId;
            quantities[itemCount] = quantity;
            unitPrices[itemCount] = unitPrice;

            itemCount++;
        }
    }

    if (ferror(cartFile))
    {
        fclose(cartFile);
        printf("Error reading cart file.\n");
        return;
    }

    fclose(cartFile);

    if (itemCount == 0)
    {
        printf("Your cart is empty.\n");
        return;
    }

    /* Validate stock */
    for (int i = 0; i < itemCount; i++)
    {
        if (!checkStockAvailability(productIds[i],
                                    quantities[i]))
        {
            printf("Insufficient stock for Product ID %d.\n",
                   productIds[i]);
            return;
        }
    }

    /* Calculate total */
    order_process(quantities, itemCount,
                  unitPrices, &totalAmount);

    if (totalAmount <= 0.0f)
    {
        printf("Invalid order total.\n");
        return;
    }

    printf("\n========================================\n");
    printf("                CHECKOUT                \n");
    printf("========================================\n");
    printf("Customer : %s\n", username);
    printf("Products : %d\n", itemCount);
    printf("Total    : Rs. %.2f\n", totalAmount);

    /* Retrieve registered customer details */
    if (!getCustomerDetails(username,
                            phoneNumber, sizeof(phoneNumber),
                            address, sizeof(address)))
    {
        printf("Unable to retrieve customer details.\n");
        return;
    }

    printf("\nPhone   : %s\n", phoneNumber);
    printf("Address : %s\n", address);

    /* Confirm order */
    char confirmation;

    printf("\nConfirm order? (y/n): ");

    if (scanf(" %c", &confirmation) != 1)
    {
        while (getchar() != '\n' && !feof(stdin))
        {
        }

        return;
    }

    while (getchar() != '\n' && !feof(stdin))
    {
    }

    if (confirmation != 'y' && confirmation != 'Y')
    {
        printf("Checkout cancelled.\n");
        return;
    }

    /* Recheck stock */
    for (int i = 0; i < itemCount; i++)
    {
        if (!checkStockAvailability(productIds[i],
                                    quantities[i]))
        {
            printf("Stock changed. Order cancelled.\n");
            return;
        }
    }

    /* Save order */
    if (!placeOrder((char *)username,
                    phoneNumber,
                    address,
                    itemCount,
                    totalAmount))
    {
        printf("Order failed. Cart has been preserved.\n");
        return;
    }

    /*
       Update stock.
       NOTE: updateStockAfterPurchase() returns void,
       so its success cannot currently be verified here.
    */
    for (int i = 0; i < itemCount; i++)
    {
        updateStockAfterPurchase(productIds[i],
                                 quantities[i]);
    }

    /* Clear customer's cart */
    clearCart(username);

    printf("\nOrder processing finished.\n");
    printf("Your cart has been cleared.\n");
    printf("Use My Orders to view your order history.\n");
}


int getCustomerDetails(
    const char *username,
    char *phoneNumber,
    size_t phoneSize,
    char *address,
    size_t addressSize)
{
    FILE *file = fopen(USERS_FILE, "r");

    if (file == NULL)
    {
        printf("Unable to open users file.\n");
        return 0;
    }

    char line[500];
    int found = 0;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        char *id       = strtok(line, "|");
        char *name     = strtok(NULL, "|");
        char *password = strtok(NULL, "|");
        char *email    = strtok(NULL, "|");
        char *phone    = strtok(NULL, "|");
        char *userAddress = strtok(NULL, "|");
        char *role     = strtok(NULL, "|");
        char *active   = strtok(NULL, "|");

        if (id == NULL || name == NULL ||
            password == NULL || email == NULL ||
            phone == NULL || userAddress == NULL ||
            role == NULL || active == NULL)
        {
            continue;
        }

        if (strcmp(name, username) == 0 &&
            strcmp(active, "1") == 0)
        {
            if (strlen(phone) >= phoneSize ||
                strlen(userAddress) >= addressSize)
            {
                fclose(file);
                return 0;
            }

            strcpy(phoneNumber, phone);
            strcpy(address, userAddress);

            found = 1;
            break;
        }
    }

    fclose(file);
    return found;
}