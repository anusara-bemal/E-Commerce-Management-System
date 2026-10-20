#ifndef ORDER_H
#define ORDER_H

//calculate tttottal price
void order_process(const int *cartItem, int quantity, float *unitPrice, float *totalPrice);

//conv number to string status
char *statusToText(int status);

//order
void placeOrder(char username[], char phoneNumber[], char address[],
                int itemCount, float totalAmount);

//view orders
void myOrders(char username[]);


//update order status (admin - subat aiyata)
void updateOrderStatus();

#endif