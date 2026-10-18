#ifndef CART_H
#define CART_H

void addToCart(char* username, int productId, int quantity);
void viewCart(char* username);
void removeFromCart(char* username, int productId);
void clearCart(char* username);

#endif