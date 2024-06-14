#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <stack>
#include <queue>
#include <unordered_map>
#include <vector>
#include <QVBoxLayout>

using namespace std;

// User class definition
class User {
public:
    string username;
    string password;
    bool isStaff;

    User();
    User(const string& u, const string& p, bool s);
    string serialize() const;
    static User deserialize(const string& str);
};

// Product class definition
class Product {
public:
    int code;
    string name;
    string category;
    string subCategory;
    string skinType;
    string range;
    double price;
    int quantity;

    Product();
    Product(int c, const string& n, const string& cat, const string& subCat, const string& st, const string& r, double p, int q);
    string serialize() const;
    static Product deserialize(const string& str);
};

// Order class definition
class Order {
public:
    string customerName;
    string address;
    string contact;
    string email;
    vector<Product> products;

    Order();
    Order(const string& cn, const string& a, const string& c, const string& e, const vector<Product>& p);
    string serialize() const;
    static Order deserialize(const string& str);
};

// Linked List Node for Users
struct UserNode {
    User user;
    UserNode* next;
    UserNode(const User& u) : user(u), next(nullptr) {}
};

// Linked List for Users
class UserList {
public:
    UserList() : head(nullptr) {}
    ~UserList() {
        while (head) {
            UserNode* temp = head;
            head = head->next;
            delete temp;
        }
    }
    void addUser(const User& user) {
        UserNode* newNode = new UserNode(user);
        newNode->next = head;
        head = newNode;
    }
    User* findUser(const string& username) {
        UserNode* current = head;
        while (current) {
            if (current->user.username == username) {
                return &(current->user);
            }
            current = current->next;
        }
        return nullptr;
    }

private:
    UserNode* head;
};

// BST Node for Products
struct ProductNode {
    Product product;
    ProductNode* left;
    ProductNode* right;
    ProductNode(const Product& p) : product(p), left(nullptr), right(nullptr) {}
};

// BST for Products
class ProductBST {
public:
    ProductBST() : root(nullptr) {}
    ~ProductBST() {
        clear(root);
    }
    void addProduct(const Product& product) {
        root = insert(root, product);
    }
    Product* findProduct(int code) {
        return search(root, code);
    }
    void removeProduct(int code) {
        root = remove(root, code);
    }
    void displayProducts(QVBoxLayout* layout, QWidget* parent);
    void clear(ProductNode* node);
    void displayFilteredProducts(QVBoxLayout* layout, QWidget* parent, const string& category, const string& subCategory, const string& skinType, const string& range);
    void inOrderFiltered(ProductNode* node, QVBoxLayout* layout, QWidget* parent, const string& category, const string& subCategory, const string& skinType, const string& range);

    ProductNode* root;

private:
    ProductNode* insert(ProductNode* node, const Product& product);
    Product* search(ProductNode* node, int code);
    ProductNode* remove(ProductNode* node, int code);
    ProductNode* minValueNode(ProductNode* node);
    void inOrder(ProductNode* node, QVBoxLayout* layout, QWidget* parent);
};

// Order Queue
class OrderQueue {
public:
    OrderQueue() = default;
    OrderQueue(const OrderQueue& other) {
        orders = other.orders;
    }
    OrderQueue& operator=(const OrderQueue& other) {
        if (this != &other) {
            orders = other.orders;
        }
        return *this;
    }
    void enqueue(const Order& order) {
        orders.push(order);
    }
    bool dequeue(Order& order) {
        if (orders.empty()) return false;
        order = orders.front();
        orders.pop();
        return true;
    }
    bool empty() const {
        return orders.empty();
    }

private:
    queue<Order> orders;
};

// MainWindow class definition
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addToCart(const Product& product);
private slots:
    void on_registerButton_clicked();
    void on_loginButton_clicked();
    void on_addProductButton_clicked();
    void on_editProductQuantityButton_clicked();
    void on_deleteProductButton_clicked();
    void on_displayProductsButton_clicked();
    void on_searchProductsButton_clicked();
    void on_viewOrdersButton_clicked();
    void on_viewCartButton_clicked();
    void on_identifySkinTypeButton_clicked();
    void on_logoutButton_clicked();
    void addToCartFromDisplay();
    void editProductFromDisplay();


private:
    UserList users;
    ProductBST products;
    OrderQueue orders;
    stack<Product> cart;
    User currentUser;
    bool isCurrentUserStaff;

    void saveUserToFile(const User& user);
    void loadUsersFromFile();
    void saveProductsToFile();
    void loadProductsFromFile();
    void saveOrderToFile(const Order& order);
    void loadOrdersFromFile();
    void displayProducts(bool isStaff);
    void searchProducts();
    void editProductQuantity();
    void deleteProduct();
    void viewOrders();
    void viewCart();
    void checkout();
    void showMainPage();
    void showStaffMenu();
    void showCustomerMenu();
    void showLoginScreen();

};

#endif // MAINWINDOW_H
