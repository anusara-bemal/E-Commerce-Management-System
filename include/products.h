#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "models.h"

void productMenu(void);

int getNextProductId(void);
void addProduct(void);
void viewProducts(void);
void searchProduct(void);
void updateProduct(void);
void deleteProduct(void);

int checkStockAvailability(int productId, int requiredQty);

void viewProductsAndAddToCart(const char *username);

int updateStockAfterPurchase(int productId, int purchasedQty);

#endif
