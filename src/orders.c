#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "orders.h"
#include "cart.h"
#include "products.h"
#include "ui.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

#define DATA_DIR          "data"
#define INVOICE_DIR       "data/invoices"
#define ORDERS_FILE       "data/orders.txt"
#define TEMP_ORDERS_FILE  "data/temp_orders.txt"
#define CART_DATA_FILE    "data/cart.txt"

#define LINE_SIZE          512
#define MAX_CHECKOUT_ITEMS 100
#define ORDER_FORMAT       "%d|%s|%s|%s|%d|%.2f|%d\n"

#define STATUS_PENDING 0
#define STATUS_PAID    1
#define STATUS_SHIPPED 2

#define DELIVERY_STANDARD 1
#define DELIVERY_EXPRESS  2
#define PAYMENT_COD       1

#define STANDARD_SHIPPING_FEE 100.0f
#define EXPRESS_SHIPPING_FEE  250.0f
#define FREE_SHIPPING_OVER    500.0f

#define CHOICE_INVALID (-1)
#define CHOICE_EOF     (-2)

/* ANSI colours (local names so they never clash with ui.h) */
#define OR_RESET  "\x1b[0m"
#define OR_BOLD   "\x1b[1m"
#define OR_DIM    "\x1b[2m"
#define OR_CYAN   "\x1b[36m"
#define OR_GREEN  "\x1b[32m"
#define OR_YELLOW "\x1b[33m"

/* Box drawing characters (UTF-8) */
#define BOX_H  "\xe2\x95\x90"
#define BOX_V  "\xe2\x95\x91"
#define BOX_TL "\xe2\x95\x94"
#define BOX_TR "\xe2\x95\x97"
#define BOX_ML "\xe2\x95\xa0"
#define BOX_MR "\xe2\x95\xa3"
#define BOX_BL "\xe2\x95\x9a"
#define BOX_BR "\xe2\x95\x9d"
#define INVOICE_WIDTH 48

typedef struct
{
    int id;
    char username[50];
    char phone[30];
    char address[200];
    int itemCount;
    float total;
    int status;
} OrderRecord;

typedef struct
{
    int productIds[MAX_CHECKOUT_ITEMS];
    int quantities[MAX_CHECKOUT_ITEMS];
    float unitPrices[MAX_CHECKOUT_ITEMS];
    int count;
} CheckoutItems;

typedef struct
{
    char phone[30];
    char address[200];
} ShippingInfo;

typedef enum
{
    DETAILS_BACK = 0,
    DETAILS_OK = 1,
    DETAILS_INVALID = 2
} DetailsResult;

/* ------------------------------------------------------------------ */
/*  Small helpers                                                      */
/* ------------------------------------------------------------------ */

static void makeDir(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

/* Reads one line, discards overflow, trims spaces. Returns 0 on EOF. */
static int inputLine(char *buf, size_t size)
{
    char *start = buf;
    size_t len;

    if (fgets(buf, (int)size, stdin) == NULL)
        return 0;

    if (strchr(buf, '\n') == NULL)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF)
            ;
    }

    buf[strcspn(buf, "\r\n")] = '\0';

    while (*start == ' ' || *start == '\t')
        start++;
    if (start != buf)
        memmove(buf, start, strlen(start) + 1);

    len = strlen(buf);
    while (len > 0 && (buf[len - 1] == ' ' || buf[len - 1] == '\t'))
        buf[--len] = '\0';

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

/* Prompt + read a non-negative number. CHOICE_INVALID / CHOICE_EOF on failure. */
static int readChoice(const char *prompt)
{
    char buf[32];
    int v;

    printf("%s", prompt);
    if (!inputLine(buf, sizeof(buf)))
        return CHOICE_EOF;
    return (parseInt(buf, &v) && v >= 0) ? v : CHOICE_INVALID;
}

static void invalidChoice(void)
{
    printf("Invalid choice\n");
    pressEnterToContinue();
}

/* Non-empty text with no '|' or line breaks (safe for the data files). */
static int validField(const char *s)
{
    return s != NULL && s[0] != '\0' && strpbrk(s, "|\r\n") == NULL;
}

/* Splits s in place on '|'. Keeps empty fields. Returns field count. */
static int splitFields(char *s, char **fields, int maxFields)
{
    int n = 1;

    fields[0] = s;
    for (; *s != '\0'; s++)
    {
        if (*s == '|' && n < maxFields)
        {
            *s = '\0';
            fields[n++] = s + 1;
        }
    }
    return n;
}

/* Windows rename() fails if the target exists, so remove it first. */
static int replaceFile(const char *tmp, const char *target)
{
    if (remove(target) != 0 && errno != ENOENT)
        return 0;
    return rename(tmp, target) == 0;
}

/* ------------------------------------------------------------------ */
/*  Order file format: id|user|phone|address|items|total|status        */
/* ------------------------------------------------------------------ */

static int parseOrderLine(const char *line, OrderRecord *o)
{
    char buf[LINE_SIZE];
    char *f[7];
    char *end;

    snprintf(buf, sizeof(buf), "%s", line);
    buf[strcspn(buf, "\r\n")] = '\0';

    if (splitFields(buf, f, 7) != 7)
        return 0;

    if (!parseInt(f[0], &o->id) || o->id < 1 ||
        !parseInt(f[4], &o->itemCount) ||
        !parseInt(f[6], &o->status))
        return 0;

    o->total = strtof(f[5], &end);
    if (end == f[5] || *end != '\0')
        return 0;

    snprintf(o->username, sizeof(o->username), "%s", f[1]);
    snprintf(o->phone, sizeof(o->phone), "%s", f[2]);
    snprintf(o->address, sizeof(o->address), "%s", f[3]);
    return 1;
}

static int writeOrder(FILE *f, const OrderRecord *o)
{
    return fprintf(f, ORDER_FORMAT, o->id, o->username, o->phone, o->address,
                   o->itemCount, o->total, o->status) >= 0;
}

/* ------------------------------------------------------------------ */
/*  Pricing / text helpers                                             */
/* ------------------------------------------------------------------ */

/* totalPrice = sum(quantities[i] * unitPrices[i]); 0 on any invalid value */
void order_process(const int *quantities, int itemCount,
                   float *unitPrices, float *totalPrice)
{
    if (totalPrice == NULL)
        return;

    *totalPrice = 0.0f;

    if (quantities == NULL || unitPrices == NULL || itemCount <= 0)
        return;

    for (int i = 0; i < itemCount; i++)
    {
        if (quantities[i] < 0 || unitPrices[i] < 0.0f)
        {
            *totalPrice = 0.0f;
            return;
        }
        *totalPrice += quantities[i] * unitPrices[i];
    }
}

const char *statusToText(int status)
{
    switch (status)
    {
    case STATUS_PENDING: return "PENDING";
    case STATUS_PAID:    return "PAID";
    case STATUS_SHIPPED: return "SHIPPED";
    default:             return "UNKNOWN";
    }
}

const char *deliveryMethodText(int deliveryMethod)
{
    return deliveryMethod == DELIVERY_EXPRESS ? "EXPRESS" : "STANDARD";
}

const char *paymentMethodText(int paymentMethod)
{
    (void)paymentMethod;
    return "CASH ON DELIVERY";
}

float getDeliveryFee(int deliveryMethod, float subtotal)
{
    if (deliveryMethod == DELIVERY_EXPRESS)
        return EXPRESS_SHIPPING_FEE;
    if (subtotal >= FREE_SHIPPING_OVER)
        return 0.0f;
    return STANDARD_SHIPPING_FEE;
}

/* ------------------------------------------------------------------ */
/*  Place order                                                        */
/* ------------------------------------------------------------------ */

/* Next free order ID, or -1 on error. */
static int generateOrderId(void)
{
    FILE *file = fopen(ORDERS_FILE, "r");
    char line[LINE_SIZE];
    int maxId = 0;
    int readError;

    if (file == NULL)
        return errno == ENOENT ? 1 : -1;

    while (fgets(line, sizeof(line), file) != NULL)
    {
        int id = atoi(line);
        if (id > maxId)
            maxId = id;
    }

    readError = ferror(file);
    fclose(file);

    if (readError || maxId == INT_MAX)
        return -1;
    return maxId + 1;
}

int placeOrder(const char *username, const char *phoneNumber,
               const char *address, int itemCount,
               float totalAmount, int *newOrderId)
{
    FILE *file;
    int newId;
    int ok;

    if (!validField(username) || !validField(phoneNumber) || !validField(address))
    {
        printf("\n[Error] Invalid customer details!\n");
        return 0;
    }

    if (itemCount <= 0 || !(totalAmount > 0.0f) || !isfinite(totalAmount))
    {
        printf("\n[Error] Invalid order details!\n");
        return 0;
    }

    makeDir(DATA_DIR);

    newId = generateOrderId();
    if (newId < 0)
    {
        printf("\n[Error] Could not generate an order ID!\n");
        return 0;
    }

    file = fopen(ORDERS_FILE, "a");
    if (file == NULL)
    {
        printf("\n[Error] Could not open %s: %s\n", ORDERS_FILE, strerror(errno));
        return 0;
    }

    ok = fprintf(file, ORDER_FORMAT, newId, username, phoneNumber, address,
                 itemCount, totalAmount, STATUS_PENDING) >= 0;
    if (fclose(file) != 0)
        ok = 0;

    if (!ok)
    {
        printf("\n[Error] Failed to save the order!\n");
        return 0;
    }

    if (newOrderId != NULL)
        *newOrderId = newId;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  View / manage orders                                               */
/* ------------------------------------------------------------------ */

void myOrders(char username[])
{
    FILE *file;
    char line[LINE_SIZE];
    OrderRecord o;
    int found = 0;

    if (username == NULL || username[0] == '\0')
    {
        showError("Invalid username!");
        pressEnterToContinue();
        return;
    }

    clearScreen();
    drawHeader("MY ORDERS");

    file = fopen(ORDERS_FILE, "r");
    if (file == NULL)
    {
        printf("\n");
        showInfo("You have no orders yet. Start shopping!");
        pressEnterToContinue();
        return;
    }

    // Table Header - Nordic Style
    printf("\n");
    printTableHeader(5, "ID", "ADDRESS", "ITEMS", "TOTAL", "STATUS");
    printTableDivider(5);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parseOrderLine(line, &o) && strcmp(o.username, username) == 0)
        {
            char idStr[10];
            char itemsStr[10];
            char totalStr[20];
            
            snprintf(idStr, sizeof(idStr), "%d", o.id);
            snprintf(itemsStr, sizeof(itemsStr), "%d", o.itemCount);
            snprintf(totalStr, sizeof(totalStr), "Rs.%.2f", o.total);
            
            // Status color එක
            const char* statusText = statusToText(o.status);
            char coloredStatus[40];
            if (o.status == 0) // Pending
                snprintf(coloredStatus, sizeof(coloredStatus), YELLOW "%s" RESET, statusText);
            else if (o.status == 1) // Delivered
                snprintf(coloredStatus, sizeof(coloredStatus), GREEN "%s" RESET, statusText);
            else
                snprintf(coloredStatus, sizeof(coloredStatus), "%s", statusText);

            // Address එක shorten කරන්න
            char shortAddr[21];
            snprintf(shortAddr, sizeof(shortAddr), "%.20s", o.address);

            printTableRow(5, idStr, shortAddr, itemsStr, totalStr, coloredStatus);
            found++;
        }
    }
    fclose(file);

    printTableDivider(5);

    if (found == 0)
    {
        printf("\n");
        showInfo("You have no orders yet. Start shopping!");
    }
    else
    {
        printf("\n  " DIM "Total orders: %d\n" RESET, found);
    }

    drawFooter();
    pressEnterToContinue();
}
/* Prints every order. Returns count, or -1 if the file can't be opened. */
static int listAllOrders(void)
{
    FILE *file = fopen(ORDERS_FILE, "r");
    char line[LINE_SIZE];
    OrderRecord o;
    int count = 0;

    if (file == NULL)
        return -1;

    clearScreen();
    drawHeader("ALL ORDERS - ADMIN VIEW");

    printf("\n");
    printTableHeader(4, "ID", "CUSTOMER", "TOTAL", "STATUS");
    printTableDivider(4);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (parseOrderLine(line, &o))
        {
            char idStr[10];
            char totalStr[20];
            char customerShort[16];
            
            snprintf(idStr, sizeof(idStr), "%d", o.id);
            snprintf(customerShort, sizeof(customerShort), "%.15s", o.username);
            snprintf(totalStr, sizeof(totalStr), "Rs.%.2f", o.total);
            
            const char* statusText = statusToText(o.status);
            char coloredStatus[40];
            if (o.status == 0) // Pending
                snprintf(coloredStatus, sizeof(coloredStatus), YELLOW "%s" RESET, statusText);
            else if (o.status == 1) // Delivered/Completed
                snprintf(coloredStatus, sizeof(coloredStatus), GREEN "%s" RESET, statusText);
            else if (o.status == 2) // Cancelled
                snprintf(coloredStatus, sizeof(coloredStatus), RED "%s" RESET, statusText);
            else
                snprintf(coloredStatus, sizeof(coloredStatus), "%s", statusText);

            printTableRow(4, idStr, customerShort, totalStr, coloredStatus);
            count++;
        }
    }
    
    printTableDivider(4);
    fclose(file);

    if (count == 0)
    {
        printf("\n");
        showInfo("No orders found in system.");
    }
    else
    {
        printf("\n  " BOLD WHITE "Total Orders: " GREEN "%d\n" RESET, count);
    }

    drawFooter();
    return count;
}
/* 1 = updated, -1 = order not found, 0 = file error.
   Unparseable lines are copied through untouched. */
static int setOrderStatus(int orderId, int newStatus)
{
    FILE *in = fopen(ORDERS_FILE, "r");
    FILE *out;
    char line[LINE_SIZE];
    OrderRecord o;
    int found = 0, ok = 1;

    if (in == NULL)
        return 0;

    out = fopen(TEMP_ORDERS_FILE, "w");
    if (out == NULL)
    {
        fclose(in);
        return 0;
    }

    while (ok && fgets(line, sizeof(line), in) != NULL)
    {
        if (!parseOrderLine(line, &o))
        {
            ok = fputs(line, out) != EOF;
            continue;
        }

        if (o.id == orderId)
        {
            o.status = newStatus;
            found = 1;
        }
        ok = writeOrder(out, &o);
    }

    if (ferror(in))
        ok = 0;
    fclose(in);
    if (fclose(out) != 0)
        ok = 0;

    if (!ok || !found)
    {
        remove(TEMP_ORDERS_FILE);
        return ok ? -1 : 0;
    }

    return replaceFile(TEMP_ORDERS_FILE, ORDERS_FILE) ? 1 : 0;
}

/* Admin: list all orders, then change one order's status */
void updateOrderStatus(void)
{
    int count = listAllOrders();
    int orderId, newStatus, r;

    if (count < 0)
    {
        printf("\n");
        showError("No orders found or orders file is unavailable.");
        pressEnterToContinue();
        return;
    }
    if (count == 0)
    {
        printf("\n");
        showInfo("No valid orders found.");
        pressEnterToContinue();
        return;
    }

    orderId = readChoice("\n" CYAN "❯" RESET " Order ID (0 = cancel): ");
    if (orderId == 0 || orderId == CHOICE_EOF)
        return;
    if (orderId < 0)
    {
        showError("Invalid order ID!");
        pressEnterToContinue();
        return;
    }

    printf("\n  " BOLD WHITE "Select New Status:\n" RESET);
    printf("  " MAGENTA "╭────────────────────────────────╮\n" RESET);
    printf("  " MAGENTA "│" RESET " " YELLOW "0" RESET " = PENDING   " DIM "(waiting)" RESET "  " MAGENTA "│\n" RESET);
    printf("  " MAGENTA "│" RESET " " GREEN "1" RESET " = PAID      " DIM "(completed)" RESET " " MAGENTA "│\n" RESET);
    printf("  " MAGENTA "│" RESET " " CYAN "2" RESET " = SHIPPED   " DIM "(on the way)" RESET " " MAGENTA "│\n" RESET);
    printf("  " MAGENTA "╰────────────────────────────────╯\n" RESET);

    newStatus = readChoice("  " CYAN "❯" RESET " Enter status: ");
    if (newStatus < STATUS_PENDING || newStatus > STATUS_SHIPPED)
    {
        showError("Invalid status! Choose 0, 1 or 2.");
        pressEnterToContinue();
        return;
    }

    r = setOrderStatus(orderId, newStatus);
    printf("\n");
    if (r > 0)
    {
        char msg[100];
        snprintf(msg, sizeof(msg), "Order %d status updated to %s!", orderId, statusToText(newStatus));
        showSuccess(msg);
    }
    else if (r < 0)
    {
        char msg[80];
        snprintf(msg, sizeof(msg), "Order ID %d was not found!", orderId);
        showError(msg);
    }
    else
    {
        char msg[120];
        snprintf(msg, sizeof(msg), "Failed to update orders file '%s'", ORDERS_FILE);
        showError(msg);
    }
    
    pressEnterToContinue();
}
/* ------------------------------------------------------------------ */
/*  Invoice                                                            */
/* ------------------------------------------------------------------ */

typedef struct
{
    int orderId;
    const char *username;
    const char *phone;
    const char *address;
    int itemCount;
    const int *productIds;
    const int *quantities;
    const float *unitPrices;
    float total;
    float shippingFee;
    int deliveryMethod;
    int paymentMethod;
    char date[40];
    char tracking[40];
} InvoiceData;

static int saveInvoiceFile(const InvoiceData *d)
{
    char path[300];
    FILE *f;

    makeDir(DATA_DIR);
    makeDir(INVOICE_DIR);
    snprintf(path, sizeof(path), "%s/invoice_%d.txt", INVOICE_DIR, d->orderId);

    f = fopen(path, "w");
    if (f == NULL)
    {
        showWarning("Could not create invoice file.");
        printf("  " DIM "%s\n" RESET, path);
        return 0;
    }

    fprintf(f, "┌────────────────────────────────────────────────┐\n");
    fprintf(f, "│           S SHOP E-COMMERCE - INVOICE          │\n");
    fprintf(f, "└────────────────────────────────────────────────┘\n");
    fprintf(f, "\n");
    fprintf(f, "  Invoice No : #%06d\n", d->orderId);
    fprintf(f, "  Date       : %s\n", d->date);
    fprintf(f, "  Tracking   : %s\n", d->tracking);
    fprintf(f, "\n");
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  CUSTOMER DETAILS\n");
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  Name       : %s\n", d->username);
    fprintf(f, "  Phone      : %s\n", d->phone);
    fprintf(f, "  Address    : %s\n", d->address);
    fprintf(f, "\n");
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  ORDER ITEMS (%d items)\n", d->itemCount);
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  %-6s %-6s %-10s %-12s\n", "ID", "QTY", "UNIT", "LINE TOTAL");
    fprintf(f, "  ──────────────────────────────────────────────\n");
    for (int i = 0; i < d->itemCount; i++)
        fprintf(f, "  #%-5d x%-4d Rs.%-7.2f Rs.%-8.2f\n", 
                d->productIds[i],
                d->quantities[i], 
                d->unitPrices[i],
                d->quantities[i] * d->unitPrices[i]);
    fprintf(f, "  ──────────────────────────────────────────────\n");
    fprintf(f, "\n");
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  PAYMENT SUMMARY\n");
    fprintf(f, "──────────────────────────────────────────────────\n");
    fprintf(f, "  Subtotal      : Rs. %8.2f\n", d->total - d->shippingFee);
    fprintf(f, "  Shipping      : Rs. %8.2f\n", d->shippingFee);
    fprintf(f, "                  ──────────\n");
    fprintf(f, "  TOTAL         : Rs. %8.2f\n", d->total);
    fprintf(f, "\n");
    fprintf(f, "  Delivery      : %s\n", deliveryMethodText(d->deliveryMethod));
    fprintf(f, "  Payment       : %s\n", paymentMethodText(d->paymentMethod));
    fprintf(f, "\n");
    fprintf(f, "┌────────────────────────────────────────────────┐\n");
    fprintf(f, "│     Thank you for shopping with NORDIC!        │\n");
    fprintf(f, "│         www.S-shop.com                   │\n");
    fprintf(f, "└────────────────────────────────────────────────┘\n");

    int ok = fclose(f) == 0;
    if (ok)
    {
        printf("\n");
        char msg[150];
        snprintf(msg, sizeof(msg), "Invoice saved: invoice_%d.txt", d->orderId);
        showSuccess(msg);
        printf("  " DIM "%s\n" RESET, path);
    }
    return ok;
}static void boxEdge(const char *left, const char *right)
{
    printf(OR_CYAN "%s", left);
    for (int i = 0; i < INVOICE_WIDTH; i++)
        printf(BOX_H);
    printf("%s" OR_RESET "\n", right);
}

static void boxRowf(const char *color, const char *fmt, ...)
{
    char text[160];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);

    printf(OR_CYAN BOX_V OR_RESET "%s%-*.*s" OR_RESET OR_CYAN BOX_V OR_RESET "\n",
           color, INVOICE_WIDTH, INVOICE_WIDTH, text);
}

static void printInvoiceBox(const InvoiceData *d)
{
    boxEdge(BOX_TL, BOX_TR);
    boxRowf(OR_BOLD OR_GREEN, "   ORDER CONFIRMED");
    boxEdge(BOX_ML, BOX_MR);
    boxRowf("", "  Invoice  : #%d", d->orderId);
    boxRowf("", "  Date     : %s", d->date);
    boxRowf(OR_YELLOW, "  Tracking : %s", d->tracking);
    boxEdge(BOX_ML, BOX_MR);
    boxRowf("", "  Customer : %s", d->username);
    boxRowf("", "  Phone    : %s", d->phone);
    boxRowf("", "  Address  : %.33s", d->address);
    boxEdge(BOX_ML, BOX_MR);
    for (int i = 0; i < d->itemCount; i++)
        boxRowf("", "  #%-5d x%-3d @ Rs.%-8.2f = Rs.%.2f", d->productIds[i],
                d->quantities[i], d->unitPrices[i],
                d->quantities[i] * d->unitPrices[i]);
    boxEdge(BOX_ML, BOX_MR);
    boxRowf("", "  Subtotal : Rs. %.2f", d->total - d->shippingFee);
    if (d->shippingFee > 0.0f)
        boxRowf("", "  Shipping : Rs. %.2f", d->shippingFee);
    else
        boxRowf(OR_GREEN, "  Shipping : FREE");
    boxRowf(OR_BOLD, "  Total    : Rs. %.2f", d->total);
    boxRowf("", "  Delivery : %s", deliveryMethodText(d->deliveryMethod));
    boxRowf("", "  Payment  : %s", paymentMethodText(d->paymentMethod));
    boxEdge(BOX_ML, BOX_MR);
    boxRowf(OR_GREEN, "      Thank you for shopping with us!");
    boxEdge(BOX_BL, BOX_BR);
}

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
    InvoiceData d;
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    d.orderId = orderId;
    d.username = username;
    d.phone = phoneNumber;
    d.address = address;
    d.itemCount = itemCount;
    d.productIds = productIds;
    d.quantities = quantities;
    d.unitPrices = unitPrices;
    d.total = totalAmount;
    d.shippingFee = shippingFee;
    d.deliveryMethod = deliveryMethod;
    d.paymentMethod = paymentMethod;

    if (tm != NULL)
        strftime(d.date, sizeof(d.date), "%d-%m-%Y %H:%M", tm);
    else
        snprintf(d.date, sizeof(d.date), "-");
    snprintf(d.tracking, sizeof(d.tracking), "TRK%06d", orderId);

    saveInvoiceFile(&d);
    printInvoiceBox(&d);
}

/* ------------------------------------------------------------------ */
/*  Customer details (users file)                                      */
/*  Format: id|name|password|email|phone|address|role|active           */
/* ------------------------------------------------------------------ */

int getCustomerDetails(const char *username,
                       char *phoneNumber, size_t phoneSize,
                       char *address, size_t addressSize)
{
    FILE *file = fopen(USERS_FILE, "r");
    char line[LINE_SIZE];
    char *f[8];
    int found = 0;

    if (file == NULL)
    {
        printf("Unable to open users file.\n");
        return 0;
    }

    while (!found && fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\r\n")] = '\0';

        if (splitFields(line, f, 8) != 8)
            continue;

        if (strcmp(f[1], username) == 0 && strcmp(f[7], "1") == 0 &&
            f[4][0] != '\0' && f[5][0] != '\0' &&
            strlen(f[4]) < phoneSize && strlen(f[5]) < addressSize)
        {
            snprintf(phoneNumber, phoneSize, "%s", f[4]);
            snprintf(address, addressSize, "%s", f[5]);
            found = 1;
        }
    }

    fclose(file);
    return found;
}

/* ------------------------------------------------------------------ */
/*  Checkout                                                           */
/* ------------------------------------------------------------------ */

/* Returns the first product ID with too little stock, or 0 if all OK. */
static int findShortStock(const CheckoutItems *it)
{
    for (int i = 0; i < it->count; i++)
        if (!checkStockAvailability(it->productIds[i], it->quantities[i]))
            return it->productIds[i];
    return 0;
}

/* Loads this customer's cart (merging duplicate products). 1 = ok. */
static int loadCartItems(const char *username, CheckoutItems *it)
{
    FILE *file = fopen(CART_DATA_FILE, "r");
    char line[LINE_SIZE];
    const char *error = NULL;

    it->count = 0;

    if (file == NULL)
    {
        printf("Unable to open cart file.\n");
        return 0;
    }

    while (error == NULL && fgets(line, sizeof(line), file) != NULL)
    {
        char cartUser[50], productName[50];
        int productId, quantity, i;
        float unitPrice, subtotal;

        if (sscanf(line, "%49[^|]|%d|%49[^|]|%d|%f|%f",
                   cartUser, &productId, productName,
                   &quantity, &unitPrice, &subtotal) != 6 ||
            strcmp(cartUser, username) != 0)
            continue;

        if (productId <= 0 || quantity <= 0 || unitPrice <= 0.0f)
        {
            error = "Invalid item found in your cart.";
            break;
        }

        for (i = 0; i < it->count; i++)
        {
            if (it->productIds[i] == productId)
                break;
        }

        if (i < it->count)
        {
            if (it->quantities[i] > INT_MAX - quantity)
                error = "Cart quantity is too large.";
            else
                it->quantities[i] += quantity;
        }
        else if (it->count >= MAX_CHECKOUT_ITEMS)
        {
            error = "Too many products in your cart.";
        }
        else
        {
            it->productIds[i] = productId;
            it->quantities[i] = quantity;
            it->unitPrices[i] = unitPrice;
            it->count++;
        }
    }

    if (error == NULL && ferror(file))
        error = "Error reading cart file.";
    fclose(file);

    if (error != NULL)
    {
        printf("%s\n", error);
        return 0;
    }
    if (it->count == 0)
    {
        printf("Your cart is empty.\n");
        return 0;
    }
    return 1;
}

/* Step 1. Returns 1 = chosen, 0 = back. */
static int selectDelivery(float subtotal, int *method)
{
    for (;;)
    {
        int c;

        clearScreen();
        drawHeader("CHECKOUT");
        printf(OR_DIM "  Cart total: Rs. %.2f" OR_RESET "\n\n", subtotal);
        printf("DELIVERY METHOD\n\n");
        printf("1. Standard Shipping - Rs. %.2f (5-8 days)", STANDARD_SHIPPING_FEE);
        if (subtotal >= FREE_SHIPPING_OVER)
            printf("  -> FREE for this order!");
        printf("\n2. Express Shipping - Rs. %.2f (2-4 days)\n", EXPRESS_SHIPPING_FEE);
        printf("\n0. Back\n");

        c = readChoice("Select: ");
        if (c == 0 || c == CHOICE_EOF)
            return 0;
        if (c == DELIVERY_STANDARD || c == DELIVERY_EXPRESS)
        {
            *method = c;
            return 1;
        }
        invalidChoice();
    }
}

static DetailsResult enterNewDetails(ShippingInfo *s)
{
    printf("\nPhone (empty = back): ");
    if (!inputLine(s->phone, sizeof(s->phone)) || s->phone[0] == '\0')
        return DETAILS_BACK;

    printf("Address: ");
    if (!inputLine(s->address, sizeof(s->address)))
        return DETAILS_BACK;

    return (validField(s->phone) && validField(s->address))
               ? DETAILS_OK : DETAILS_INVALID;
}

/* Step 2. Returns 1 = details chosen, 0 = back. */
static int selectShipping(int hasSaved, const ShippingInfo *saved,
                          ShippingInfo *chosen, float subtotal, float fee)
{
    for (;;)
    {
        DetailsResult r;
        clearScreen();
        drawHeader("CHECKOUT - SHIPPING");

        // Price summary card
        printf("\n  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
          printf("  " MAGENTA "│" RESET "  Subtotal: " WHITE "Rs. %-8.2f" RESET " Shipping: " YELLOW "Rs. %-6.2f" RESET " " MAGENTA "│\n" RESET, subtotal, fee);
          printf("  " MAGENTA "│" RESET "  Total:    " BOLD GREEN "Rs. %-8.2f" RESET "                         " MAGENTA "│\n" RESET, subtotal + fee);
          printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

        printf("\n  " BOLD WHITE "SHIPPING DETAILS\n\n" RESET);

        if (hasSaved)
        {
            int c;

            printf("  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
            printf("  " MAGENTA "│" RESET " " DIM "SAVED ADDRESS" RESET "                              " MAGENTA "│\n" RESET);
            printf("  " MAGENTA "│" RESET "  Phone   : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, saved->phone);
            printf("  " MAGENTA "│" RESET "  Address : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, saved->address);
            printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

            printf("\n  " CYAN "1" RESET ". Use saved details " DIM "(quick checkout)" RESET "\n");
            printf("  " CYAN "2" RESET ". Enter new shipping details\n");
            printf("  " RED "0" RESET ". Back to cart\n");

            c = readChoice("\n  " CYAN "❯" RESET " Select: ");
            if (c == 0 || c == CHOICE_EOF)
                return 0;
            if (c == 1)
            {
                *chosen = *saved;
                showSuccess("Using saved shipping details.");
                return 1;
            }
            if (c != 2)
            {
                invalidChoice();
                continue;
            }
        }

        r = enterNewDetails(chosen);
        if (r == DETAILS_OK)
            return 1;
        if (r == DETAILS_BACK)
        {
            if (hasSaved)
                continue;
            return 0;
        }

        showError("Invalid details (address is required, '|' is not allowed)");
        pressEnterToContinue();
    }
}
/* Step 3. Returns 1 = cash on delivery chosen, 0 = back. */
static int selectPayment(float grandTotal)
{
    for (;;)
    {
        int c;
        clearScreen();
        drawHeader("CHECKOUT - PAYMENT");

        // Grand Total Card
        printf("\n  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
          printf("  " MAGENTA "│" RESET "  Amount to Pay: " BOLD GREEN "Rs. %-10.2f" RESET "              " MAGENTA "│\n" RESET, grandTotal);
          printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

        printf("\n  " BOLD WHITE "PAYMENT METHOD\n\n" RESET);

        printf("  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
        printf("  " MAGENTA "│" RESET "  " BOLD CYAN "1" RESET ". " WHITE "Cash on Delivery" RESET "                      " MAGENTA "│\n" RESET);
        printf("  " MAGENTA "│" RESET "     " DIM "Pay when you receive the order" RESET "      " MAGENTA "│\n" RESET);
        printf("  " MAGENTA "│" RESET "     " GREEN "✓ No extra charges" RESET "                    " MAGENTA "│\n" RESET);
        printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

        printf("\n  " RED "0" RESET ". Back\n");

        c = readChoice("\n  " CYAN "❯" RESET " Select: ");
        if (c == 0 || c == CHOICE_EOF)
            return 0;
        if (c == PAYMENT_COD)
        {
            showSuccess("Cash on Delivery selected.");
            return 1;
        }
        invalidChoice();
    }
}
static int confirmOrder(const char *username, const ShippingInfo *s,
                        float subtotal, float fee, int deliveryMethod)
{
    char answer[8];

    clearScreen();
    drawHeader("CHECKOUT - CONFIRMATION");

    printf("\n  " BOLD WHITE "ORDER SUMMARY\n" RESET);
    printf("  " MAGENTA "╭──────────────────────────────────────────╮\n" RESET);
    printf("  " MAGENTA "│" RESET " Customer : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, username);
    printf("  " MAGENTA "│" RESET " Phone    : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, s->phone);
    printf("  " MAGENTA "│" RESET " Address  : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, s->address);
    printf("  " MAGENTA "│" RESET " " DIM "──────────────────────────────────────" RESET " " MAGENTA "│\n" RESET);
    printf("  " MAGENTA "│" RESET " Subtotal : " WHITE "Rs. %-8.2f" RESET "                  " MAGENTA "│\n" RESET, subtotal);
    printf("  " MAGENTA "│" RESET " Shipping : " YELLOW "Rs. %-8.2f" RESET "                  " MAGENTA "│\n" RESET, fee);
    printf("  " MAGENTA "│" RESET " Delivery : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, deliveryMethodText(deliveryMethod));
    printf("  " MAGENTA "│" RESET " Payment  : " CYAN "%-28.28s" RESET " " MAGENTA "│\n" RESET, paymentMethodText(PAYMENT_COD));
    printf("  " MAGENTA "│" RESET "          " DIM "──────────────────────" RESET "          " MAGENTA "│\n" RESET);
    printf("  " MAGENTA "│" RESET " " BOLD "TOTAL    : " GREEN "Rs. %-8.2f" RESET "                  " MAGENTA "│\n" RESET, subtotal + fee);
    printf("  " MAGENTA "╰──────────────────────────────────────────╯\n" RESET);

    printf("\n");
    printf("  " BOLD WHITE "Confirm order? " RESET DIM "(y/n): " RESET);

    if (inputLine(answer, sizeof(answer)) && (answer[0] == 'y' || answer[0] == 'Y'))
    {
        showSuccess("Order confirmed! Processing...");
        return 1;
    }

    showWarning("Checkout cancelled.");
    pressEnterToContinue();
    return 0;
}
/* Places the order, writes the invoice and reduces stock. 1 = success. */
static int submitOrder(const char *username, const CheckoutItems *it,
                       const ShippingInfo *s, float subtotal, float fee,
                       int deliveryMethod)
{
    float grandTotal = subtotal + fee;
    int orderId = 0;
    int shortId = findShortStock(it);

    if (shortId != 0)
    {
        printf("Stock changed (Product ID %d). Order cancelled.\n", shortId);
        pressEnterToContinue();
        return 0;
    }

    if (!placeOrder(username, s->phone, s->address, it->count, grandTotal, &orderId))
    {
        printf("Order failed.\n");
        pressEnterToContinue();
        return 0;
    }

    generateInvoice(orderId, username, s->phone, s->address, it->count,
                    it->productIds, it->quantities, it->unitPrices,
                    grandTotal, fee, deliveryMethod, PAYMENT_COD);

    for (int i = 0; i < it->count; i++)
    {
        if (!updateStockAfterPurchase(it->productIds[i], it->quantities[i]))
            printf(YELLOW "[Warning] Could not update stock for Product ID %d.\n" RESET,
                   it->productIds[i]);
    }

    printf("\n");
    pressEnterToContinue();
    return 1;
}

/* Delivery -> shipping -> payment -> confirm. Returns 1 if order placed. */
static int runCheckout(const char *username, CheckoutItems *items)
{
    ShippingInfo saved = {{0}, {0}};
    ShippingInfo chosen = {{0}, {0}};
    float subtotal = 0.0f, fee = 0.0f;
    int deliveryMethod = DELIVERY_STANDARD;
    int step = 1;
    int hasSaved;
    int shortId = findShortStock(items);

    if (shortId != 0)
    {
        printf("Insufficient stock for Product ID %d.\n", shortId);
        return 0;
    }

    order_process(items->quantities, items->count, items->unitPrices, &subtotal);
    if (subtotal <= 0.0f)
    {
        printf("Invalid order total.\n");
        return 0;
    }

    hasSaved = getCustomerDetails(username, saved.phone, sizeof(saved.phone),
                                  saved.address, sizeof(saved.address));

    for (;;)
    {
        if (step == 1)
        {
            if (!selectDelivery(subtotal, &deliveryMethod))
                return 0;
            fee = getDeliveryFee(deliveryMethod, subtotal);
            step = 2;
        }
        else if (step == 2)
        {
            step = selectShipping(hasSaved, &saved, &chosen, subtotal, fee) ? 3 : 1;
        }
        else
        {
            if (!selectPayment(subtotal + fee))
            {
                step = 2;
                continue;
            }
            if (!confirmOrder(username, &chosen, subtotal, fee, deliveryMethod))
                return 0;
            return submitOrder(username, items, &chosen, subtotal, fee, deliveryMethod);
        }
    }
}

/* Checkout everything in the customer's cart. */
void checkout(const char *username)
{
    CheckoutItems items;

    if (!validField(username))
    {
        printf("Invalid customer account.\n");
        return;
    }

    if (!loadCartItems(username, &items))
        return;

    if (runCheckout(username, &items))
        clearCart(username);
}

/* "Buy now": checkout a single product, cart is left untouched. */
void startDirectCheckout(const char *username, int productId,
                         int quantity, float unitPrice)
{
    CheckoutItems items;

    if (!validField(username) || productId <= 0 || quantity <= 0 ||
        !(unitPrice > 0.0f))
    {
        printf("Invalid checkout request.\n");
        return;
    }

    items.count = 1;
    items.productIds[0] = productId;
    items.quantities[0] = quantity;
    items.unitPrices[0] = unitPrice;

    runCheckout(username, &items);
}