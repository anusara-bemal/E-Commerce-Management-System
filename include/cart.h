#ifndef CART_H
#define CART_H

#include "models.h"

void addToCart(const char *username, int productId, int quantity);
void viewCart(const char *username);
void updateCartQuantity(const char *username);
void removeFromCart(const char *username, int productId);
void clearCart(const char *username);

#endif
