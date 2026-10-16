# E-Commerce Management System

## 1. Project Overview

The E-Commerce Management System is a Command Line Interface (CLI) application developed using the C programming language.

The system allows customers to register, log in, browse products, manage their shopping carts, place orders, and view their order history. Administrators can manage products, customers, and orders through a dedicated admin panel.

This project is developed as a first-year university C programming group project.

## 2. Project Objectives

* Develop a functional CLI-based e-commerce system using C.
* Demonstrate fundamental C programming concepts.
* Implement file handling for persistent data storage.
* Apply structures, arrays, functions, loops, and conditional statements.
* Practice modular programming and collaborative development using GitHub.
* Implement basic customer and administrator operations.

## 3. Technologies Used

* **Programming Language:** C
* **Compiler:** GCC
* **Interface:** Command Line Interface (CLI)
* **Data Storage:** Text or binary files
* **Version Control:** Git and GitHub
* **Development Environment:** Visual Studio Code

## 4. User Roles

### Customer

Customers will be able to:

* Register a new account.
* Log in to an existing account.
* View available products.
* Search for products.
* Add products to a shopping cart.
* Remove items from the cart.
* Proceed to checkout.
* View previous orders.
* View their profile.
* Log out.

### Administrator

Administrators will be able to:

* Log in using administrator credentials.
* Add, view, search, update, and delete products.
* View and search customer records.
* View customer details and manage customer records.
* View and search orders.
* Update order statuses.
* Cancel orders.
* Generate basic system reports.

## 5. Project Structure

```text
E-Commerce-Management-System/
│
├── src/
│   ├── main.c
│   ├── users.c
│   ├── products.c
│   ├── cart.c
│   ├── orders.c
│   └── admin.c
│
├── include/
│   ├── models.h
│   ├── users.h
│   ├── products.h
│   ├── cart.h
│   ├── orders.h
│   └── admin.h
│
├── data/
│   ├── users.dat
│   ├── products.dat
│   ├── orders.dat
│   └── cart.dat
│
├── run.bat
├── README.md
└── .gitignore
```

### Directory Description

* `src/` — Contains the C source files.
* `include/` — Contains header files, structures, and function declarations.
* `data/` — Stores application data using files.
* `run.bat` — Compiles and runs the application on Windows when GCC is installed.
* `README.md` — Documents the project.

## 6. Data Storage

The application uses file handling in C to store information between program executions.

| File           | Purpose                                                      |
| -------------- | ------------------------------------------------------------ |
| `users.dat`    | Stores customer account information.                         |
| `products.dat` | Stores product information and stock quantities.             |
| `orders.dat`   | Stores order records and order statuses.                     |
| `cart.dat`     | Stores cart information if persistent carts are implemented. |

No database management system is used.

The implementation may use functions such as:

* `fopen()`
* `fclose()`
* `fprintf()`
* `fscanf()`
* `fread()`
* `fwrite()`
* `fseek()`
* `remove()`
* `rename()`

The selected file format must be used consistently throughout the application.

## 7. Main Menu

The application starts with the following menu:

```text
========================================
      E-COMMERCE MANAGEMENT SYSTEM
========================================

1. Customer Login
2. Customer Registration
3. Admin Login
4. Exit

Enter your choice:
```

Customers and administrators are directed to their respective menus after successful authentication.

## 8. Installation and Execution

### Requirements

Install the following software:

* GCC C compiler
* Visual Studio Code or another text editor
* Windows operating system for the supplied `run.bat` script

### Run on Windows

1. Install GCC and ensure it is available through your system's PATH.
2. Open the project folder.
3. Ensure all required source files are present.
4. Double-click `run.bat` or execute it from the terminal.

```powershell
.\run.bat
```

The program will compile and run if compilation succeeds.

### Compile Manually

From the project root directory, run:

```bash
gcc -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c -o ecommerce.exe
```

Run on Windows:

```powershell
.\ecommerce.exe
```

On Linux, compile the same source files using:

```bash
gcc -Iinclude src/main.c src/users.c src/products.c src/cart.c src/orders.c src/admin.c -o ecommerce
```

Run on Linux:

```bash
./ecommerce
```

**Note:** These commands assume that the source files and their required function implementations exist. The project must be integrated before the complete application can compile successfully.

## 9. C Programming Concepts

The project demonstrates the following concepts:

* Variables and data types
* Conditional statements (`if`, `else`, `switch`)
* Loops (`for`, `while`, `do-while`)
* Functions and modular programming
* Arrays and strings
* Structures (`struct`)
* Pointers
* File handling
* Input validation
* Error handling
* Basic searching and updating algorithms

## 10. Development Workflow

The project is developed collaboratively using GitHub.

Recommended workflow:

1. Clone the project repository.
2. Create a separate branch for your assigned module.
3. Implement and test your functions.
4. Commit your changes with a meaningful message.
5. Push your branch to GitHub.
6. Create a pull request.
7. Review and integrate changes into the main branch.
8. Compile and test the integrated application.

Team members should agree on shared structures, function names, parameter types, and file formats before integrating their modules.

## 11. Testing

The application should be tested for:

* Successful customer registration and login.
* Duplicate usernames.
* Invalid credentials.
* Invalid menu selections.
* Product searching and updating.
* Insufficient product stock.
* Shopping cart calculations.
* Order placement and stock reduction.
* Order status updates.
* File opening and writing errors.
* Data persistence after restarting the application.

## 12. Security and Data Integrity

* Validate user input before processing it.
* Prevent duplicate usernames.
* Check stock availability before placing an order.
* Verify that prices and quantities are valid.
* Check file operations for errors.
* Restrict administrator operations to authenticated administrators.
* Avoid storing real customer information in test files.
* Treat plain-text password storage as a prototype limitation; real applications require appropriate password hashing.

## 13. Project Scope and Limitations

This project is an educational prototype.

It does not include:

* A graphical user interface.
* A web interface.
* A database management system.
* Real online payment processing.
* External APIs.
* Online shipping integration.

Orders and payments, if included, are simulated within the CLI application.

## 14. Future Improvements

Possible improvements include:

* Better input validation.
* Improved file error handling.
* More detailed sales reports.
* Product category filtering.
* Persistent shopping carts.
* Improved order tracking.
* Stronger authentication practices.

## 15. Contributors

This project is developed by a six-member university team.

| Member   | Responsibility |
| -------- | -------------- |
| Member 1 | To be assigned |
| Member 2 | To be assigned |
| Member 3 | To be assigned |
| Member 4 | To be assigned |
| Member 5 | To be assigned |
| Member 6 | To be assigned |

Update the responsibilities according to the team's final task allocation.

## 16. License

This project is developed for educational purposes as part of a university programming course.
