#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <direct.h>
#include <time.h>
#include <sys/stat.h>
#include "orders.h"
#include "cart.h"
#include "products.h"
#include "ui.h"

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
    float totalAmount,
    int *newOrderId)
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

    if (newOrderId != NULL)
    {
        *newOrderId = newId;
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

clearScreen();
drawHeader("MY ORDERS");


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

// --------------------------------------------------
// Delivery options and payment options
// --------------------------------------------------
#define DELIVERY_STANDARD 1
#define DELIVERY_EXPRESS  2

#define PAYMENT_COD  1

#define STANDARD_SHIPPING_FEE  100.0f
#define EXPRESS_SHIPPING_FEE  250.0f
#define FREE_SHIPPING_OVER     500.0f

const char *deliveryMethodText(int deliveryMethod)
{
    switch (deliveryMethod)
    {
        case DELIVERY_EXPRESS:
            return "EXPRESS";

        case DELIVERY_STANDARD:
        default:
            return "STANDARD";
    }
}

const char *paymentMethodText(int paymentMethod)
{
    (void)paymentMethod;
    return "CASH ON DELIVERY";
}

float getDeliveryFee(int deliveryMethod, float subtotal)
{
    if (deliveryMethod == DELIVERY_EXPRESS)
    {
        return EXPRESS_SHIPPING_FEE;
    }

    // Free standard shipping for orders over Rs. 500/-
    if (subtotal >= FREE_SHIPPING_OVER)
    {
        return 0.0f;
    }

    return STANDARD_SHIPPING_FEE;
}

// --------------------------------------------------
// Checkout Step 1: Customer chooses delivery method
// --------------------------------------------------
int chooseDeliveryMethod(float subtotal)
{
    printf("\nDelivery Options:\n");
    printf("1. Standard Shipping - Rs. %.2f (Delivery in 5-8 days)\n", STANDARD_SHIPPING_FEE);
    printf("2. Express Shipping - Rs. %.2f (Delivery in 2-4 days)\n", EXPRESS_SHIPPING_FEE);

    if (subtotal >= FREE_SHIPPING_OVER)
    {
        printf("Free Standard Shipping is available for orders over Rs. %.2f!\n", FREE_SHIPPING_OVER);
    }

    int choice = 0;
    int valid = 0;

    while (valid == 0)
    {
        printf("Enter your delivery choice (1/2): ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input!\n");

            while (getchar() != '\n' && !feof(stdin))
            {
            }

            choice = 0;
        }

        while (getchar() != '\n' && !feof(stdin))
        {
        }

        if (choice == DELIVERY_STANDARD || choice == DELIVERY_EXPRESS)
        {
            valid = 1;
        }
        else
        {
            printf("Invalid choice! Please select 1 or 2.\n");
        }
    }

    return choice;
}

// Checkout Step 2: Payment - Always Cash on Delivery
int choosePaymentMethod(void)
{
    printf("\nPayment Options:\n");
    printf("1. Cash on Delivery (Pay when your order arrives)\n");

    int choice = 0;
    int valid = 0;

    while (valid == 0)
    {
        printf("Enter your payment choice (1): ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input!\n");

            while (getchar() != '\n' && !feof(stdin))
            {
            }

            choice = 0;
        }

        while (getchar() != '\n' && !feof(stdin))
        {
        }

        if (choice == PAYMENT_COD)
        {
            valid = 1;
        }
        else
        {
            printf("Invalid choice! Please select 1.\n");
        }
    }

    return choice;
}

// --------------------------------------------------
// Generate invoice after placing an order
// --------------------------------------------------
void generateInvoice(int orderId,
                     const char *username,
                     const char *phoneNumber,
                     const char *address,
                     int itemCount,
                     const int *productIds,
                     const int *quantities,
                     const float *unitPrices,
                     float totalAmount,
                     float shippingFee,
                     int deliveryMethod,
                     int paymentMethod)
{
    const char *invoiceDir = "data/invoices";
    const char *invoiceFileName = "invoice";
    char invoicePath[300];

    sprintf(invoicePath, "%s/%s_%d.txt", invoiceDir, invoiceFileName, orderId);

#ifdef _WIN32
    _mkdir(invoiceDir);
#else
    mkdir(invoiceDir, 0755);
#endif

    FILE *file = fopen(invoicePath, "w");

    if (file == NULL)
    {
        printf("\n[Error] Could not create invoice file!\n");
        printf("Invoice path: %s\n", invoicePath);
        return;
    }

    time_t now = time(NULL);
    char dateStr[40];
    strftime(dateStr, sizeof(dateStr), "%d-%m-%Y %H:%M", localtime(&now));

    char trackingNumber[40];
    sprintf(trackingNumber, "TRK%06d", orderId);

    fprintf(file, "==============================================\n");
    fprintf(file, "                      INVOICE                      \n");
    fprintf(file, "==============================================\n");
    fprintf(file, "Invoice No   : %d\n", orderId);
    fprintf(file, "Date         : %s\n", dateStr);
    fprintf(file, "Customer     : %s\n", username);
    fprintf(file, "Phone        : %s\n", phoneNumber);
    fprintf(file, "Address      : %s\n", address);
    fprintf(file, "----------------------------------------------\n");

    for (int i = 0; i < itemCount; i++)
    {
        float subtotal = quantities[i] * unitPrices[i];

        fprintf(file, "Product ID : %d\n", productIds[i]);
        fprintf(file, "Quantity   : %d\n", quantities[i]);
        fprintf(file, "Unit Price : Rs. %.2f\n", unitPrices[i]);
        fprintf(file, "Subtotal   : Rs. %.2f\n", subtotal);
        fprintf(file, "----------------------------------------------\n");
    }

    fprintf(file, "Subtotal     : Rs. %.2f\n", totalAmount - shippingFee);
    fprintf(file, "Shipping     : Rs. %.2f\n", shippingFee);
    fprintf(file, "Total        : Rs. %.2f\n", totalAmount);
    fprintf(file, "Delivery     : %s\n", deliveryMethodText(deliveryMethod));
    fprintf(file, "Payment      : %s\n", paymentMethodText(paymentMethod));
    fprintf(file, "Tracking No  : %s\n", trackingNumber);
    fprintf(file, "==============================================\n");
    fprintf(file, "Thank you for your order!\n");

    fclose(file);

    // Print the invoice to the screen
    printf("\n==============================================\n");
    printf("                      INVOICE                      \n");
    printf("==============================================\n");
    printf("Invoice No   : %d\n", orderId);
    printf("Date         : %s\n", dateStr);
    printf("Customer     : %s\n", username);
    printf("Phone        : %s\n", phoneNumber);
    printf("Address      : %s\n", address);
    printf("----------------------------------------------\n");

    for (int i = 0; i < itemCount; i++)
    {
        float subtotal = quantities[i] * unitPrices[i];

        printf("Product ID : %d\n", productIds[i]);
        printf("Quantity   : %d\n", quantities[i]);
        printf("Unit Price : Rs. %.2f\n", unitPrices[i]);
        printf("Subtotal   : Rs. %.2f\n", subtotal);
        printf("----------------------------------------------\n");
    }

    printf("Subtotal     : Rs. %.2f\n", totalAmount - shippingFee);
    printf("Shipping     : Rs. %.2f\n", shippingFee);
    printf("Total        : Rs. %.2f\n", totalAmount);
    printf("Delivery     : %s\n", deliveryMethodText(deliveryMethod));
    printf("Payment      : %s\n", paymentMethodText(paymentMethod));
    printf("Tracking No  : %s\n", trackingNumber);
    printf("==============================================\n");
    printf("Thank you for your order!\n");
}



//checkout directly
static int pendingProductId = -1;
static int pendingQuantity = 0;
static float pendingUnitPrice = 0.0f;

void startDirectCheckout(const char *username,
                         int productId,
                         int quantity,
                         float unitPrice)
{
    pendingProductId = productId;
    pendingQuantity = quantity;
    pendingUnitPrice = unitPrice;

    checkout(username);
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

    if (pendingProductId > 0)
    {
        productIds[0] = pendingProductId;
        quantities[0] = pendingQuantity;
        unitPrices[0] = pendingUnitPrice;
        itemCount = 1;
    }
    else
    {
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
    }

    //Validate stock
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



   //Calculate total
    order_process(quantities, itemCount,
                  unitPrices, &totalAmount);

    if (totalAmount <= 0.0f)
    {
        printf("Invalid order total.\n");
        return;
    }

    clearScreen();
    drawHeader("CHECKOUT");
    printf("Customer : %s\n", username);
    printf("Items    : %d\n\n", itemCount);

    for (int i = 0; i < itemCount; i++)
    {
        printf("%d. Product ID : %d | Qty : %d | Unit : Rs. %.2f | Subtotal : Rs. %.2f\n",
               i + 1, productIds[i], quantities[i], unitPrices[i], quantities[i] * unitPrices[i]);
    }
    printf("\n----------------------------------------\n");
    printf("Subtotal : Rs. %.2f\n", totalAmount);

    int step = 1;
    int deliveryMethod = 1;
    float shippingFee = 0.0f;
    float grandTotal = totalAmount;

    char savedPhone[30] = {0};
    char savedAddress[200] = {0};
    int hasSavedDetails = getCustomerDetails(username,
                                             savedPhone, sizeof(savedPhone),
                                             savedAddress, sizeof(savedAddress));

    phoneNumber[0] = '\0';
    address[0] = '\0';

    while (step == 1)
    {
        clearScreen();
        drawHeader("CHECKOUT");
        printf("Customer : %s | Subtotal : Rs. %.2f\n\n", username, totalAmount);
        printf("[STEP 1] DELIVERY\n\n");
        printf("1. Standard Shipping - Rs. %.2f (5-8 days)\n", STANDARD_SHIPPING_FEE);
        printf("2. Express Shipping - Rs. %.2f (2-4 days)\n", EXPRESS_SHIPPING_FEE);
        if (totalAmount >= FREE_SHIPPING_OVER)
            printf("(Free Standard over Rs. %.2f)\n", FREE_SHIPPING_OVER);
        printf("\n0. Back\n");
        printf("Select: ");

        int choice = -1;
        if (scanf("%d", &choice) != 1)
        {
            while (getchar() != '\n' && !feof(stdin)) {}
            continue;
        }
        while (getchar() != '\n' && !feof(stdin)) {}

        if (choice == 0) return;
        if (choice == 1 || choice == 2)
        {
            deliveryMethod = choice;
            shippingFee = getDeliveryFee(deliveryMethod, totalAmount);
            grandTotal = totalAmount + shippingFee;
            step = 2;
            continue;
        }
        printf("Invalid choice\n");
        printf("Press Enter to continue...");
        while (getchar() != '\n' && !feof(stdin)) {}
    }

    while (step == 2)
    {
        clearScreen();
        drawHeader("CHECKOUT");
        printf("Customer : %s | Subtotal : Rs. %.2f | Shipping : Rs. %.2f | Total : Rs. %.2f\n\n", username, totalAmount, shippingFee, grandTotal);
        printf("[STEP 2] SHIPPING DETAILS\n\n");
        if (hasSavedDetails)
        {
            printf("Saved Phone   : %s\n", savedPhone);
            printf("Saved Address : %s\n\n", savedAddress);
            printf("1. Use saved details\n");
            printf("2. Enter new shipping details\n");
            printf("\n0. Back\n");
            printf("Select: ");

            int sc = -1;
            if (scanf("%d", &sc) != 1)
            {
                while (getchar() != '\n' && !feof(stdin)) {}
                continue;
            }
            while (getchar() != '\n' && !feof(stdin)) {}

            if (sc == 0) { step = 1; continue; }
            if (sc == 1)
            {
                strcpy(phoneNumber, savedPhone);
                strcpy(address, savedAddress);
                step = 3;
                continue;
            }
            if (sc == 2)
            {
                printf("\nPhone: ");
                if (scanf("%29s", phoneNumber) != 1) { while (getchar()!='\n'&&!feof(stdin)){} phoneNumber[0]='\0'; }
                while (getchar() != '\n' && !feof(stdin)) {}
                printf("Address: ");
                if (scanf(" %199[^\n]", address) != 1) { while (getchar()!='\n'&&!feof(stdin)){} address[0]='\0'; }
                while (getchar() != '\n' && !feof(stdin)) {}
                if (phoneNumber[0] && address[0]) { step = 3; continue; }
                printf("Invalid details\n");
                printf("Press Enter to continue...");
                while (getchar() != '\n' && !feof(stdin)) {}
                continue;
            }
            printf("Invalid choice\n");
            printf("Press Enter to continue...");
            while (getchar() != '\n' && !feof(stdin)) {}
            continue;
        }
        else
        {
            printf("Phone: ");
            if (scanf("%29s", phoneNumber) != 1) { while (getchar()!='\n'&&!feof(stdin)){} phoneNumber[0]='\0'; }
            while (getchar() != '\n' && !feof(stdin)) {}
            printf("Address: ");
            if (scanf(" %199[^\n]", address) != 1) { while (getchar()!='\n'&&!feof(stdin)){} address[0]='\0'; }
            while (getchar() != '\n' && !feof(stdin)) {}
            if (phoneNumber[0] && address[0]) { step = 3; continue; }
            printf("\n0. Back\n");
            printf("Select 0 to go back or retry: ");
            int bx = -1;
            if (scanf("%d",&bx)==1 && bx==0){while(getchar()!='\n'&&!feof(stdin)){} step=1; continue;}
            while (getchar() != '\n' && !feof(stdin)) {}
            continue;
        }
    }

    while (step == 3)
    {
        clearScreen();
        drawHeader("CHECKOUT");
        printf("Customer : %s | Total : Rs. %.2f\n\n", username, grandTotal);
        printf("[STEP 3] PAYMENT\n\n");
        printf("1. Cash on Delivery (Pay on delivery)\n");
        printf("\n0. Back\n");
        printf("Select: ");
        int pc = -1;
        if (scanf("%d",&pc)!=1){ while(getchar()!='\n'&&!feof(stdin)){} continue;}
        while(getchar()!='\n'&&!feof(stdin)){}
        if (pc == 0){ step = 2; continue; }
        if (pc == 1)
        {
            int paymentMethod = PAYMENT_COD;
            clearScreen();
            drawHeader("CHECKOUT");
            printf("ORDER SUMMARY\n\n");
            printf("Customer : %s\n", username);
            printf("Phone    : %s\n", phoneNumber);
            printf("Address  : %s\n\n", address);
            printf("Subtotal : Rs. %.2f\n", totalAmount);
            printf("Shipping : Rs. %.2f\n", shippingFee);
            printf("Total    : Rs. %.2f\n", grandTotal);
            printf("Delivery : %s\n", deliveryMethodText(deliveryMethod));
            printf("Payment  : CASH ON DELIVERY\n\n");
            printf("Confirm order? (y/n): ");

            char confirmation;
            if (scanf(" %c", &confirmation) != 1) { while(getchar()!='\n'&&!feof(stdin)){} return; }
            while(getchar()!='\n'&&!feof(stdin)){}
            if (confirmation != 'y' && confirmation != 'Y'){ printf("Checkout cancelled.\n"); printf("Press Enter to continue..."); while(getchar()!='\n'&&!feof(stdin)){} return; }

            for (int i = 0; i < itemCount; i++)
            {
                if (!checkStockAvailability(productIds[i], quantities[i])){ printf("Stock changed. Order cancelled.\n"); printf("Press Enter to continue..."); while(getchar()!='\n'&&!feof(stdin)){} return; }
            }

            int newOrderId = 0;
            if (!placeOrder((char *)username, phoneNumber, address, itemCount, grandTotal, &newOrderId)){ printf("Order failed.\n"); printf("Press Enter to continue..."); while(getchar()!='\n'&&!feof(stdin)){} return; }

            generateInvoice(newOrderId, (char *)username, phoneNumber, address, itemCount, productIds, quantities, unitPrices, grandTotal, shippingFee, deliveryMethod, paymentMethod);

            for (int i = 0; i < itemCount; i++) updateStockAfterPurchase(productIds[i], quantities[i]);
            clearCart(username);
            printf("\nOrder placed.\n");
            printf("Press Enter to continue...");
            while(getchar()!='\n'&&!feof(stdin)){}
            return;
        }
        printf("Invalid choice\n");
        printf("Press Enter to continue...");
        while (getchar() != '\n' && !feof(stdin)) {}
    }
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