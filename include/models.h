#ifndef MODELS_H
#define MODELS_H

#define MAX_USERNAME 30
#define MAX_EMAIL 50
#define MAX_PRODUCT_NAME 100
#define MAX_PASSWORD 30
#define MAX_CART_ITEMS 50

#define USERS_FILE "data/users.txt"
#define PRODUCTS_FILE "data/products.txt"
#define ORDERS_FILE "data/orders.txt"
#define CART_FILE "data/cart.txt" 

typedef enum {
    ROLE_CUSTOMER = 0,
    ROLE_ADMIN = 1
} UserRole;

typedef enum {
    ORDER_PENDING = 0,
    ORDER_COMPLETED = 1,
    ORDER_CANCELLED = 2
} OrderStatus;

typedef struct {
    int id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char email[MAX_EMAIL];
    UserRole role;
    int isActive; 
} User;

typedef struct {
    int id;
    char name[MAX_PRODUCT_NAME];
    float price;
    int stock;
    int isActive;
} Product;

typedef struct {
    int productId;
    int quantity;
} CartItem;

typedef struct {
    int userId; 
    CartItem items[MAX_CART_ITEMS];
    int itemCount;
} Cart;

typedef struct {
    int orderId;
    int userId;
    float totalAmount;
    OrderStatus status;
} Order;

#endif
//hello