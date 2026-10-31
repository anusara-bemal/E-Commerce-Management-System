# E-Commerce Management System

A Command Line Interface (CLI) e-commerce application written in C.

Customers can register, log in, browse and search products, manage a shopping cart, place orders and view their order history. Administrators can manage products, users and orders, and view a simple dashboard.

This is a first-year university C programming group project.

## Table of Contents

1. [Features](#1-features)
2. [Technologies Used](#2-technologies-used)
3. [Project Structure](#3-project-structure)
4. [Building and Running](#4-building-and-running)
5. [How to Use](#5-how-to-use)
6. [Data Storage](#6-data-storage)
7. [C Concepts Demonstrated](#7-c-concepts-demonstrated)
8. [Limitations](#8-limitations)
9. [Team Members](#9-team-members)

## 1. Features

### Guest

* View the product list without logging in.
* Read the in-app instructions and About Us page.

### Customer

* Register a new account and log in.
* Browse products page by page and add them to the cart.
* Search for products.
* View the cart, update item quantities, remove items or clear the cart.
* Checkout with a choice of delivery method:
  * Standard shipping: Rs. 100.00 (5-8 days), free when the cart total is Rs. 500.00 or more.
  * Express shipping: Rs. 250.00 (2-4 days).
* Pay with Cash on Delivery, after confirming the order summary.
* Receive an invoice, saved as a text file in `data/invoices/`.
* View previous orders.
* Stock is checked before an order is placed and reduced after purchase.

### Administrator

* Add, update and delete products.
* User management: view all users, delete a user, promote a user to admin or demote an admin to a normal user.
* Update order status (`PENDING`, `PAID`, `SHIPPED`).
* Dashboard with a system overview (product, user and order counts, total stock and stock value), product statistics and an inventory report (low stock and out of stock items).

## 2. Technologies Used

| Item | Details |
| --- | --- |
| Language | C |
| Compiler | GCC (MinGW-w64 for Windows builds) |
| Interface | Command Line Interface with ANSI colours and Unicode box characters |
| Data storage | Plain text files, `|` delimited |
| Version control | Git and GitHub |
| Editor | Visual Studio Code |

## 3. Project Structure

```text
E-Commerce-Management-System/
│
├── src/
│   ├── main.c        Main menu, customer login/registration, admin panel loop
│   ├── ui.c          Colours, headers, menus and message helpers
│   ├── input.c       Safe input reading and validation
│   ├── users.c       Login, registration, user roles
│   ├── products.c    Product add, update, delete, browse and search
│   ├── cart.c        Cart operations
│   ├── orders.c      Checkout, invoices, order history, order status
│   └── admin.c       User management and dashboard
│
├── include/          Header files for each module (models.h holds the data structures)
│
├── data/
│   ├── users.txt     User accounts
│   ├── product.txt   Products and stock
│   ├── orders.txt    Orders
│   ├── cart.txt      Cart items
│   └── invoices/     One invoice file per order (created automatically)
│
├── assets/
│   └── icon.ico      Icon used for the Windows executable
│
├── icon.rc           Resource file that embeds the icon in the .exe
├── ecommerce.exe     Windows executable
├── README.md
└── .gitignore
```

## 4. Building and Running

**Always run the program from the project root folder.** The program opens its data files using relative paths such as `data/users.txt`, so it cannot find them if it is started from another folder.

### Windows (GCC / MinGW-w64 installed)

From the project root:

```powershell
gcc -Wall -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c src/input.c src/ui.c -o ecommerce.exe
.\ecommerce.exe
```

To embed the icon, build the resource file first and add `icon.o` to the command:

```powershell
windres icon.rc -O coff -o icon.o
gcc -Wall -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c src/input.c src/ui.c icon.o -o ecommerce.exe
```

Use Windows Terminal (or a recent Windows 10/11 console) for correct colours. The program sets the console to UTF-8 automatically.

### Windows executable built from Linux or WSL

```bash
sudo apt install mingw-w64
x86_64-w64-mingw32-windres icon.rc -O coff -o icon.o
x86_64-w64-mingw32-gcc -static -Wall -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c src/input.c src/ui.c icon.o -o ecommerce.exe
```

The `-static` option makes the executable run on other Windows PCs without extra DLL files.

### Linux

```bash
gcc -Wall -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c src/input.c src/ui.c -o ecommerce
./ecommerce
```

Every `.c` file in `src/` must be included in the build command.

## 5. How to Use

The main menu:

```text
1. Customer Login
2. Customer Registration
3. View Products (Guest)
4. Instructions
5. About Us
99. Admin Login
0. Exit
```

### Customer flow

1. Choose `2` to register, then `1` to log in.
2. Use **View Products & Buy** or **Search Product** to find items and add them to the cart.
3. Open **My Cart** to review items, change quantities or remove items, then check out.
4. Choose the delivery method and payment method, and confirm the order.
5. The invoice is saved as `data/invoices/invoice_<order id>.txt`.
6. Use **My Orders** to see your order history.

### Administrator flow

1. Choose `99` at the main menu and log in with an account whose role is admin.
2. Use the admin panel to manage products, users and orders, or open the dashboard.

Default administrator account: username `admin`, password `1234`. Change this if the data files are replaced.

## 6. Data Storage

All data is stored in text files inside `data/`. One record is stored per line, with fields separated by `|`. No database is used.

| File | Purpose |
| --- | --- |
| `users.txt` | User accounts |
| `product.txt` | Products and stock quantities |
| `orders.txt` | Order records and statuses |
| `cart.txt` | Items currently in customers' carts |
| `invoices/` | One generated invoice per order |

`users.txt` format:

```text
id|username|password|email|phone|address|role|isActive
```

`role` is `0` for a customer and `1` for an administrator.

`product.txt` format:

```text
id|name|category|price|stock|description
```

Files are updated by writing a temporary file and then replacing the original, so records are not lost if a write fails part-way.

## 7. C Concepts Demonstrated

* Variables, data types and `enum`
* Conditional statements and loops
* Functions and modular programming with header files
* Arrays and strings
* Structures (`struct`)
* Pointers
* File handling (`fopen`, `fgets`, `fprintf`, `remove`, `rename`)
* Input validation and error handling
* Searching, updating and deleting records
* Variadic functions for table printing

## 8. Limitations

This project is an educational prototype.

* Passwords are stored as plain text. A real application must hash passwords.
* Payment is simulated. Only Cash on Delivery is available.
* There is no graphical or web interface, and no database.
* Orders cannot be cancelled. The statuses are `PENDING`, `PAID` and `SHIPPED`.
* Data files are plain text, so they can be edited by hand and are not protected against concurrent use.

## 9. Team Members

| Student ID | Name | Responsibility |
| --- | --- | --- |
| TG/IT/2025/2522 | H.G.L.R. Prasanna (Leader) | Main function and UI |
| TG/IT/2025/2484 | A.B. Manamperi | Cart |
| TG/IT/2025/2504 | A.R.S. Aluthwaththa | Admin part |
| TG/IT/2025/2476 | G. Salitha Devin Perera | User management |
| TG/IT/2025/2481 | H.G.S. Dilshan | Product section |
| TG/IT/2025/2477 | N.H.S.T. Nimsara | Order section |

## License

Developed for educational purposes as part of a university programming course.