#ifndef ORDERS_H
#define ORDERS_H

#include <stddef.h>

void order_process(
    const int *cartItem,
    int quantity,
    float *unitPrice,
    float *totalPrice
);

const char *statusToText(int status);

int placeOrder(
    const char *username,
    const char *phoneNumber,
    const char *address,
    int itemCount,
    float totalAmount
);

void myOrders(char username[]);

void updateOrderStatus(void);

void checkout(const char *username);

int getCustomerDetails(
    const char *username,
    char *phoneNumber,
    size_t phoneSize,
    char *address,
    size_t addressSize
);

#endif

