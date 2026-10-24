#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "models.h"

void productMenu(void);

void addProduct(void);
void viewProducts(void);
void searchProduct(void);
void updateProduct(void);
void deleteProduct(void);

int checkStockAvailability(int productId, int requiredQty);
void updateStockAfterPurchase(int productId, int purchasedQty);

#endif
