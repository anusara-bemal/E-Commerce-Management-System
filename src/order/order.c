#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "order.h"


#define ORDERS_FILE "data/orders.txt"
#define MAX_LINE 300


//calculate total price
void order_process(const int *cartItem, int quantity, float *unitPrice, float *totalPrice) {
    *totalPrice = 0.0f;

    for (int i = 0; i < quantity; i++) {
        *totalPrice += cartItem[i] * unitPrice[i];
    }
}



//status number to string convert
char *statusToText(int status) {
    if (status == 0) {
        return "PENDING";
    } else if (status == 1) {
        return "PAID";
    } else if (status == 2) {
        return "SHIPPED";
    } else {
        return "UNKNOWN";
    }
}




//place order
void placeOrder(char username[], char phoneNumber[], char address[],
                int itemCount, float totalAmount) {
    int newId = 1;
    FILE *readFile = fopen(ORDERS_FILE, "r");
    if (readFile != NULL) {
        char line[MAX_LINE];
        int count = 0;
        while (fgets(line, MAX_LINE, readFile) != NULL) {
            count++;
        }
        fclose(readFile);
        newId = count + 1;
    }


    FILE *writeFile = fopen(ORDERS_FILE, "a");
    if (writeFile == NULL) {
        printf("\n[Error] orders.txt open karanna bari une!\n");
        return;
    }
    fprintf(writeFile, "%d|%s|%s|%s|%d|%.2f|%d\n",
            newId, username, phoneNumber, address,
            itemCount, totalAmount, 0);
    fclose(writeFile);



    printf("\n[OK] Order placed!\n");
    printf("Order ID   = %d\n", newId);
    printf("Total      = %.2f\n", totalAmount);
    printf("Status     = PENDING\n");
}


//view orders
void myOrders(char username[]) {
    FILE *file = fopen(ORDERS_FILE, "r");

    if (file == NULL) {
        printf("\nYou have no orders yet.\n");
        return;
    }

    printf("\n===== MY ORDERS =====\n");
    printf("%-6s %-18s %-6s %-9s\n", "ID", "ADDRESS", "ITEMS", "STATUS");

    char line[MAX_LINE];
    int found = 0;

    while (fgets(line, MAX_LINE, file) != NULL) {
        int id, itemCount, status;
        float total;
        char name[30], phone[15], address[100];
        sscanf(line, "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
               &id, name, phone, address, &itemCount, &total, &status);

        if (strcmp(name, username) == 0) {
            printf("%-6d %-18s %-6d %-9s\n", id, address, itemCount, statusToText(status));
            found++;
        }
    }
    fclose(file);

    if (found == 0) {
        printf("\nYou have no orders yet.\n");
    }
}


//updatte order status (for to subat aiya)
void updateOrderStatus() {
    printf("\n===== ALL ORDERS =====\n");
    printf("%-6s %-15s %-8s %-9s\n", "ID", "CUSTOMER", "TOTAL", "STATUS");

    FILE *file = fopen(ORDERS_FILE, "r");
    if (file == NULL) {
        printf("\nNo orders yet.\n");
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, file) != NULL) {
        int id, itemCount, status;
        float total;
        char name[30], phone[15], address[100];

        sscanf(line, "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
               &id, name, phone, address, &itemCount, &total, &status);

        printf("%-6d %-15s %-8.2f %-9s\n", id, name, total, statusToText(status));
    }
    fclose(file);

    int orderId, newStatus;
    printf("\nOrder ID (0 = cancel) : ");
    scanf("%d", &orderId);
    if (orderId == 0) {
        return;
    }

    printf("New Status  0=PENDING  1=PAID  2=SHIPPED : ");
    scanf("%d", &newStatus);
    FILE *oldFile = fopen(ORDERS_FILE, "r");
    FILE *tempFile = fopen("data/temp_orders.txt", "w");
    if (oldFile == NULL || tempFile == NULL) {
        printf("\n[Error] File open karanna bari une!\n");
        return;
    }
    while (fgets(line, MAX_LINE, oldFile) != NULL) {
        int id, itemCount, status;
        float total;
        char name[30], phone[15], address[100];
        sscanf(line, "%d|%29[^|]|%14[^|]|%99[^|]|%d|%f|%d",
               &id, name, phone, address, &itemCount, &total, &status);

        if (id == orderId) {
            status = newStatus;
        }
        fprintf(tempFile, "%d|%s|%s|%s|%d|%.2f|%d\n",
                id, name, phone, address, itemCount, total, status);
    }
    fclose(oldFile);
    fclose(tempFile);
    remove(ORDERS_FILE);
    rename("data/temp_orders.txt", ORDERS_FILE);

    printf("\n[OK] Order %d status updated!\n", orderId);
}