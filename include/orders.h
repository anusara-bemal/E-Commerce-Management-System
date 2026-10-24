#ifndef ORDER_H
#define ORDER_H

// Calculate total price
void order_process(const int *cartItem, int quantity,
float *unitPrice, float *totalPrice);

// Convert status number to text
const char *statusToText(int status);

// Place an order
void placeOrder(char username[], char phoneNumber[], char address[],
int itemCount, float totalAmount);

// View orders belonging to a customer
void myOrders(char username[]);

// View all orders and update order status
void updateOrderStatus(void);

#endif
