#ifndef MODELS_H
#define MODELS_H

#define MAX_USERNAME 30
#define MAX_EMAIL 50
//PRODUCT
#define MAX_PRODUCT_NAME 50
#define MAX_CATEGORY 30
#define DESCRIPTION 100


#define MAX_PASSWORD 30
#define MAX_tpNumber 20
#define MAX_Adress 300
#define MAX_CART_ITEMS 50

#define DELIMITER "|"
#define DELIMITER_CHAR '|'

#define USERS_FILE "data/users.txt"
#define PRODUCTS_FILE "data/product.txt"
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
    char tpNumber[MAX_tpNumber];
    char address[MAX_Adress];
    UserRole role;//role eka 1 kiyanne admin 0 kiyanne user 
    int isActive;
} User;



typedef struct {
    int productId;
    char productName[50];
    char category[30];
    float price;
    int stockQuantity;
    char description[100];
} Product;

typedef struct {
    char customerName[50];
    int productId;
    char productName[50];
    int quantity;
    float unitPrice;
    float subtotal;
} CartItem;
typedef struct {
    int userId; 
    CartItem items[MAX_CART_ITEMS];
    int itemCount;
} Cart;

typedef struct {
    int orderId;
    char customerName[MAX_USERNAME];
    char phoneNumber[MAX_tpNumber];
    char address[MAX_Adress];
    int itemcount;
    int discountAmount;
    int userId;
    float totalAmount;
    OrderStatus status;
} Order;

#endif
//hello