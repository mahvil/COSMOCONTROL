#include "mainwindow.h"
#include "identify_skin_type.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QTextEdit>
#include <fstream>
#include <QUrl>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <list>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QScrollArea>

using namespace std;

// User class implementation
User::User() : username(""), password(""), isStaff(false) {}

User::User(const string& u, const string& p, bool s) : username(u), password(p), isStaff(s) {}

string User::serialize() const {
    return username + "," + password + "," + (isStaff ? "1" : "0") + "\n";
}

User User::deserialize(const string& str) {
    size_t pos1 = str.find(',');
    size_t pos2 = str.find(',', pos1 + 1);
    if (pos1 == string::npos || pos2 == string::npos || pos2 + 1 >= str.size()) {
        throw invalid_argument("Malformed input string for User deserialization");
    }
    string u = str.substr(0, pos1);
    string p = str.substr(pos1 + 1, pos2 - pos1 - 1);
    bool s = str[pos2 + 1] == '1';
    return User(u, p, s);
}

// Product class implementation
Product::Product() : code(0), name(""), category(""), subCategory(""), skinType(""), range(""), price(0.0), quantity(0) {}

Product::Product(int c, const string& n, const string& cat, const string& subCat, const string& st, const string& r, double p, int q)
    : code(c), name(n), category(cat), subCategory(subCat), skinType(st), range(r), price(p), quantity(q) {}

string Product::serialize() const {
    return to_string(code) + "," + name + "," + category + "," + subCategory + "," + skinType + "," + range + "," + to_string(price) + "," + to_string(quantity) + "\n";
}

Product Product::deserialize(const string& str) {
    size_t pos1 = str.find(',');
    size_t pos2 = str.find(',', pos1 + 1);
    size_t pos3 = str.find(',', pos2 + 1);
    size_t pos4 = str.find(',', pos3 + 1);
    size_t pos5 = str.find(',', pos4 + 1);
    size_t pos6 = str.find(',', pos5 + 1);
    size_t pos7 = str.find(',', pos6 + 1);

    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos ||
        pos4 == string::npos || pos5 == string::npos || pos6 == string::npos || pos7 == string::npos) {
        throw invalid_argument("Malformed input string for Product deserialization");
    }

    int c = stoi(str.substr(0, pos1));
    string n = str.substr(pos1 + 1, pos2 - pos1 - 1);
    string cat = str.substr(pos2 + 1, pos3 - pos2 - 1);
    string subCat = str.substr(pos3 + 1, pos4 - pos3 - 1);
    string st = str.substr(pos4 + 1, pos5 - pos4 - 1);
    string r = str.substr(pos5 + 1, pos6 - pos5 - 1);
    double p = stod(str.substr(pos6 + 1, pos7 - pos6 - 1));
    int q = stoi(str.substr(pos7 + 1));

    return Product(c, n, cat, subCat, st, r, p, q);
}

// Order class implementation
Order::Order() {}

Order::Order(const string& cn, const string& a, const string& c, const string& e, const vector<Product>& p)
    : customerName(cn), address(a), contact(c), email(e), products(p) {}

string Order::serialize() const {
    string serializedProducts;
    for (const auto& product : products) {
        serializedProducts += product.serialize();
    }
    return customerName + "," + address + "," + contact + "," + email + "\n" + serializedProducts;
}

Order Order::deserialize(const string& str) {
    size_t pos1 = str.find(',');
    size_t pos2 = str.find(',', pos1 + 1);
    size_t pos3 = str.find(',', pos2 + 1);
    size_t pos4 = str.find('\n', pos3 + 1);

    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || pos4 == string::npos) {
        throw invalid_argument("Malformed input string for Order deserialization");
    }

    string cn = str.substr(0, pos1);
    string a = str.substr(pos1 + 1, pos2 - pos1 - 1);
    string c = str.substr(pos2 + 1, pos3 - pos2 - 1);
    string e = str.substr(pos3 + 1, pos4 - pos3 - 1);

    vector<Product> products;
    size_t startPos = pos4 + 1;
    while (startPos < str.size()) {
        size_t endPos = str.find('\n', startPos);
        if (endPos == string::npos) {
            endPos = str.size();
        }
        string productStr = str.substr(startPos, endPos - startPos);
        try {
            products.push_back(Product::deserialize(productStr));
        } catch (const invalid_argument& e) {
            cerr << "Error deserializing product: " << e.what() << endl;
        }
        startPos = endPos + 1;
    }

    Order order(cn, a, c, e, products);
    // Debug statement to check deserialization
    qDebug() << "Deserialized Order:" << QString::fromStdString(order.serialize());
    return order;
}

// MainWindow class implementation
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isCurrentUserStaff(false) {
    // Set up a basic UI with registration, login, and add product buttons
    QPushButton *registerButton = new QPushButton("Register", this);
    QPushButton *loginButton = new QPushButton("Login", this);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::on_registerButton_clicked);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::on_loginButton_clicked);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(registerButton);
    topLayout->addWidget(loginButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);

    QPushButton *displayProductsButton = new QPushButton("Display Products", this);
    QPushButton *searchProductsButton = new QPushButton("Search Products", this);
    QPushButton *viewCartButton = new QPushButton("View Cart", this);
    QPushButton *identifySkinTypeButton = new QPushButton("Identify Skin Type", this);

    connect(displayProductsButton, &QPushButton::clicked, this, &MainWindow::on_displayProductsButton_clicked);
    connect(searchProductsButton, &QPushButton::clicked, this, &MainWindow::on_searchProductsButton_clicked);
    connect(viewCartButton, &QPushButton::clicked, this, &MainWindow::on_viewCartButton_clicked);
    connect(identifySkinTypeButton, &QPushButton::clicked, this, &MainWindow::on_identifySkinTypeButton_clicked);

    mainLayout->addWidget(displayProductsButton);
    mainLayout->addWidget(searchProductsButton);
    mainLayout->addWidget(viewCartButton);
    mainLayout->addWidget(identifySkinTypeButton);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    loadUsersFromFile();
    loadProductsFromFile();
    loadOrdersFromFile();
    showMainPage();
}

MainWindow::~MainWindow() {}

void MainWindow::on_registerButton_clicked() {
    bool ok;
    QString username = QInputDialog::getText(this, tr("Register"),
                                             tr("Username:"), QLineEdit::Normal,
                                             "", &ok);
    if (!ok || username.isEmpty()) return;
    QMessageBox *dialog = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
    if (dialog) {
        dialog->setStyleSheet("background-color: #FFCDD2;");
    }

    QString password;
    while (true) {
        password = QInputDialog::getText(this, tr("Register"),
                                         tr("Password (at least 8 characters):"), QLineEdit::Password,
                                         "", &ok);
        if (!ok) return;
        if (password.length() >= 8) break;
        QMessageBox::warning(this, tr("Register"), tr("Password must be at least 8 characters long."));
        QMessageBox *warningBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }

    bool isStaff = QMessageBox::question(this, tr("Register"), tr("Is Staff?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
    QMessageBox *questionBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
    if (questionBox) {
        questionBox->setStyleSheet("background-color: #FFCDD2;");
    }

    if (isStaff) {
        QString staffCode = QInputDialog::getText(this, tr("Register"),
                                                  tr("Enter Staff Code:"), QLineEdit::Normal,
                                                  "", &ok);
        if (!ok || (staffCode != "mahvil" && staffCode != "ayesha")) {
            QMessageBox::warning(this, tr("Register"), tr("Invalid staff code. Registering as customer."));
            isStaff = false;
            QMessageBox *warningBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
            if (warningBox) {
                warningBox->setStyleSheet("background-color: #FFCDD2;");
            }
        }
    }

    User user(username.toStdString(), password.toStdString(), isStaff);
    users.addUser(user);
    saveUserToFile(user);

    QMessageBox::information(this, tr("Register"), tr("User registered successfully!"));
    QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
    if (infoBox) {
        infoBox->setStyleSheet("background-color: #FFCDD2;");
    }
}

void MainWindow::on_loginButton_clicked() {
    bool ok;
    QString username = QInputDialog::getText(this, tr("Login"),
                                             tr("Username:"), QLineEdit::Normal,
                                             "", &ok);
    if (!ok || username.isEmpty()) return;
    QInputDialog *dialog= dynamic_cast<QInputDialog *>(QApplication::activeWindow());
    if (dialog) {
        dialog->setStyleSheet("background-color: #FFCDD2;");
    }

    QString password = QInputDialog::getText(this, tr("Login"),
                                             tr("Password:"), QLineEdit::Password,
                                             "", &ok);
    if (!ok || password.isEmpty()) return;

    User* user = users.findUser(username.toStdString());
    if (user && user->password == password.toStdString()) {
        QMessageBox::information(this, tr("Login"), tr("Login successful!"));
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
        currentUser = *user;
        isCurrentUserStaff = currentUser.isStaff;
        if (currentUser.isStaff) {
            showStaffMenu();
        } else {
            showCustomerMenu();
        }
    } else {
        QMessageBox::warning(this, tr("Login"), tr("Invalid username or password!"));
        QMessageBox *warningBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }
}

void MainWindow::on_addProductButton_clicked() {
    bool ok;
    int code = QInputDialog::getInt(this, tr("Add Product"), tr("Product Code:"), 0, 0, 10000, 1, &ok);
    if (!ok) return;

    QString name = QInputDialog::getText(this, tr("Add Product"), tr("Product Name:"), QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString category = QInputDialog::getItem(this, tr("Add Product"), tr("Category:"), {"Skincare", "Haircare", "Makeup"}, 0, false, &ok);
    if (!ok || category.isEmpty()) return;

    QStringList subCategories;
    if (category == "Skincare") {
        subCategories << "Cleansers" << "Exfoliants" << "Toners" << "Serums" << "Moisturizers" << "Sunscreens" << "Eye Creams" << "Face Masks" << "Spot Treatments" << "Facial Oils" << "Essences" << "Face Mists" << "Lip Care" << "Anti-Aging Products" << "Acne Treatments" << "Brightening Products";
    } else if (category == "Haircare") {
        subCategories << "Shampoo" << "Conditioner" << "Hair Oil" << "Hair Mask" << "Hair Serum" << "Hair Spray" << "Hair Mousse" << "Hair Gel" << "Leave-In Conditioner" << "Hair Cream" << "Hair Wax" << "Hair Foam" << "Hair Balm" << "Hair Treatment" << "Dry Shampoo" << "Heat Protectant" << "Hair Toner" << "Hair Detangler" << "Scalp Scrub" << "Hair Fragrance";
    } else if (category == "Makeup") {
        subCategories << "Foundation" << "Concealer" << "Powder" << "Blush" << "Bronzer" << "Highlighter" << "Contour" << "Primer" << "Eyeshadow" << "Eyeliner" << "Mascara" << "Eyebrow Pencil" << "Eyebrow Gel" << "Lipstick" << "Lip Gloss" << "Lip Liner" << "Lip Balm" << "Setting Spray" << "Setting Powder" << "BB Cream" << "CC Cream" << "Tinted Moisturizer" << "Eyelash Curler" << "Face Mist" << "Makeup Remover" << "Eyebrow Powder" << "Lip Stain" << "Eyeshadow Primer" << "Lip Plumper" << "Color Corrector";
    }

    QString subCategory = QInputDialog::getItem(this, tr("Add Product"), tr("SubCategory:"), subCategories, 0, false, &ok);
    if (!ok || subCategory.isEmpty()) return;

    QString skinType = QInputDialog::getItem(this, tr("Add Product"), tr("Skin Type:"), {"Oily", "Dry", "Combination", "Sensitive", "All"}, 0, false, &ok);
    if (!ok || skinType.isEmpty()) return;

    QString range = QInputDialog::getItem(this, tr("Add Product"), tr("Price Range:"), {"Low", "Medium", "High"}, 0, false, &ok);
    if (!ok || range.isEmpty()) return;

    double price = QInputDialog::getDouble(this, tr("Add Product"), tr("Price:"), 0, 0, 10000, 2, &ok);
    if (!ok) return;

    int quantity = QInputDialog::getInt(this, tr("Add Product"), tr("Quantity:"), 0, 0, 1000, 1, &ok);
    if (!ok) return;

    Product product(code, name.toStdString(), category.toStdString(), subCategory.toStdString(), skinType.toStdString(), range.toStdString(), price, quantity);
    products.addProduct(product);
    saveProductsToFile();

    QMessageBox::information(this, tr("Add Product"), tr("Product added successfully!"));
    QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
    if (infoBox) {
        infoBox->setStyleSheet("background-color: #FFCDD2;");
    }
}

void MainWindow::on_editProductQuantityButton_clicked() {
    editProductQuantity();
}

void MainWindow::on_deleteProductButton_clicked() {
    deleteProduct();
}

void MainWindow::on_displayProductsButton_clicked() {
    displayProducts(currentUser.isStaff);
}

void MainWindow::on_searchProductsButton_clicked() {
    searchProducts();
}

void MainWindow::on_viewOrdersButton_clicked() {
    viewOrders();
}

void MainWindow::on_viewCartButton_clicked() {
    viewCart();
}

void MainWindow::on_identifySkinTypeButton_clicked() {
    identifySkinType(this);
}

void MainWindow::on_logoutButton_clicked() {
    currentUser = User();
    isCurrentUserStaff = false;
    showMainPage();
}

void MainWindow::saveUserToFile(const User& user) {
    ofstream file("users.txt", ios::app);
    if (!file.is_open()) {
        qDebug() << "Error: Unable to open users.txt for writing";
        return;
    }
    file << user.serialize();
    file.close();
    qDebug() << "User saved to file: " << QString::fromStdString(user.serialize());
}

void MainWindow::loadUsersFromFile() {
    ifstream file("users.txt");
    string line;
    while (getline(file, line)) {
        try {
            User user = User::deserialize(line);
            users.addUser(user);
            qDebug() << "User loaded from file: " << QString::fromStdString(user.serialize());
        } catch (const invalid_argument& e) {
            cerr << "Error deserializing user: " << e.what() << endl;
        }
    }
    file.close();
}

void MainWindow::saveProductsToFile() {
    ofstream file("products.txt");
    if (!file.is_open()) {
        qDebug() << "Error: Unable to open products.txt for writing";
        return;
    }

    ProductNode* current = products.root;
    stack<ProductNode*> nodes;
    while (current || !nodes.empty()) {
        while (current) {
            file << current->product.serialize();
            nodes.push(current);
            current = current->left;
        }
        current = nodes.top();
        nodes.pop();
        current = current->right;
    }

    file.close();
    qDebug() << "Products saved to file.";
}

void MainWindow::loadProductsFromFile() {
    ifstream file("products.txt");
    string line;
    while (getline(file, line)) {
        try {
            Product product = Product::deserialize(line);
            products.addProduct(product);
            qDebug() << "Product loaded from file: " << QString::fromStdString(product.serialize());
        } catch (const invalid_argument& e) {
            cerr << "Error deserializing product: " << e.what() << endl;
        }
    }
    file.close();
}

void MainWindow::saveOrderToFile(const Order& order) {
    ofstream file("orders.txt", ios::app);
    if (!file.is_open()) {
        qDebug() << "Error: Unable to open orders.txt for writing";
        return;
    }
    file << order.serialize();
    file.close();
    qDebug() << "Order saved to file: " << QString::fromStdString(order.serialize());
}

void MainWindow::loadOrdersFromFile() {
    ifstream file("orders.txt");
    string line;
    while (getline(file, line)) {
        try {
            Order order = Order::deserialize(line);
            orders.enqueue(order);
            qDebug() << "Order loaded from file: " << QString::fromStdString(order.serialize());
        } catch (const invalid_argument& e) {
            cerr << "Error deserializing order: " << e.what() << endl;
        }
    }
    file.close();
}


void MainWindow::displayProducts(bool isStaff) {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    // Add headers
    QLabel *headerLabel = new QLabel("Code\tName\tSkin Type\tPrice\tQuantity");
    headerLabel->setStyleSheet("background-color: #8B0000; color: white; padding: 5px;"); // Dark red background with white text
    layout->addWidget(headerLabel);

    // Display products
    products.displayProducts(layout, this);

    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red

    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (currentUser.username.empty()) {
            showMainPage();
        } else if (isCurrentUserStaff) {
            showStaffMenu();
        } else {
            showCustomerMenu();
        }
    });

    layout->addWidget(backButton);

    widget->setLayout(layout);
    widget->setStyleSheet("background-color: #FFCDD2;"); // Light red background

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(widget);
    scrollArea->setWidgetResizable(true);

    QWidget *container = new QWidget(this);
    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->addWidget(scrollArea);
    container->setLayout(containerLayout);

    setCentralWidget(container);
}

void ProductBST::displayProducts(QVBoxLayout* layout, QWidget* parent) {
    inOrder(root, layout, parent);
}

void MainWindow::searchProducts() {
    bool ok;
    QString category = QInputDialog::getItem(this, tr("Search Products"), tr("Category:"), {"Skincare", "Haircare", "Makeup"}, 0, false, &ok);
    if (!ok || category.isEmpty()) return;

    QStringList subCategories;
    if (category == "Skincare") {
        subCategories << "Cleansers" << "Exfoliants" << "Toners" << "Serums" << "Moisturizers" << "Sunscreens" << "Eye Creams" << "Face Masks" << "Spot Treatments" << "Facial Oils" << "Essences" << "Face Mists" << "Lip Care" << "Anti-Aging Products" << "Acne Treatments" << "Brightening Products";
    } else if (category == "Haircare") {
        subCategories << "Shampoo" << "Conditioner" << "Hair Oil" << "Hair Mask" << "Hair Serum" << "Hair Spray" << "Hair Mousse" << "Hair Gel" << "Leave-In Conditioner" << "Hair Cream" << "Hair Wax" << "Hair Foam" << "Hair Balm" << "Hair Treatment" << "Dry Shampoo" << "Heat Protectant" << "Hair Toner" << "Hair Detangler" << "Scalp Scrub" << "Hair Fragrance";
    } else if (category == "Makeup") {
        subCategories << "Foundation" << "Concealer" << "Powder" << "Blush" << "Bronzer" << "Highlighter" << "Contour" << "Primer" << "Eyeshadow" << "Eyeliner" << "Mascara" << "Eyebrow Pencil" << "Eyebrow Gel" << "Lipstick" << "Lip Gloss" << "Lip Liner" << "Lip Balm" << "Setting Spray" << "Setting Powder" << "BB Cream" << "CC Cream" << "Tinted Moisturizer" << "Eyelash Curler" << "Face Mist" << "Makeup Remover" << "Eyebrow Powder" << "Lip Stain" << "Eyeshadow Primer" << "Lip Plumper" << "Color Corrector";
    }

    QString subCategory = QInputDialog::getItem(this, tr("Search Products"), tr("SubCategory:"), subCategories, 0, false, &ok);
    if (!ok || subCategory.isEmpty()) return;

    QString skinType = QInputDialog::getItem(this, tr("Search Products"), tr("Skin Type:"), {"Oily", "Dry", "Combination", "Sensitive", "All"}, 0, false, &ok);
    if (!ok || skinType.isEmpty()) return;

    QString range = QInputDialog::getItem(this, tr("Search Products"), tr("Price Range:"), {"Low", "Medium", "High"}, 0, false, &ok);
    if (!ok || range.isEmpty()) return;

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    // Add headers
    QLabel *headerLabel = new QLabel("Code\tName\tSkin Type\tPrice\tQuantity");
    headerLabel->setStyleSheet("background-color: #8B0000; color: white; padding: 5px;"); // Dark red background with white text
    layout->addWidget(headerLabel);

    // Display filtered products
    products.displayFilteredProducts(layout, this, category.toStdString(), subCategory.toStdString(), skinType.toStdString(), range.toStdString());

    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red

    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (currentUser.username.empty()) {
            showMainPage();
        } else if (isCurrentUserStaff) {
            showStaffMenu();
        } else {
            showCustomerMenu();
        }
    });

    layout->addWidget(backButton);

    widget->setLayout(layout);
    widget->setStyleSheet("background-color: #FFCDD2;"); // Light red background
    setCentralWidget(widget);
}


void MainWindow::editProductQuantity() {
    bool ok;
    int code = QInputDialog::getInt(this, tr("Edit Product Quantity"), tr("Product Code:"), 0, 0, 10000, 1, &ok);
    if (!ok) return;

    Product* product = products.findProduct(code);
    if (product) {
        int quantity = QInputDialog::getInt(this, tr("Edit Product Quantity"), tr("New Quantity:"), product->quantity, 0, 10000, 1, &ok);
        if (!ok) return;

        product->quantity = quantity;
        saveProductsToFile();

        QMessageBox::information(this, tr("Edit Product Quantity"), tr("Product quantity updated successfully!"));
      QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
    } else {
        QMessageBox::warning(this, tr("Edit Product Quantity"), tr("Product not found!"));
        QMessageBox *warningBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }
}

void MainWindow::deleteProduct() {
    bool ok;
    int code = QInputDialog::getInt(this, tr("Delete Product"), tr("Product Code:"), 0, 0, 10000, 1, &ok);
    if (!ok) return;

    Product* product = products.findProduct(code);
    if (product) {
        products.removeProduct(code);
        saveProductsToFile();
        QMessageBox::information(this, tr("Delete Product"), tr("Product deleted successfully!"));
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
    } else {
        QMessageBox::warning(this, tr("Delete Product"), tr("Product not found!"));
        QMessageBox *warningBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }
}
void MainWindow::viewOrders() {
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    QString orderDetails;

    OrderQueue tempOrders = orders;
    while (!tempOrders.empty()) {
        Order order;
        tempOrders.dequeue(order);

        orderDetails += QString("Customer Name: %1\nAddress: %2\nContact: %3\nEmail: %4\n\nProducts:\n")
                            .arg(QString::fromStdString(order.customerName))
                            .arg(QString::fromStdString(order.address))
                            .arg(QString::fromStdString(order.contact))
                            .arg(QString::fromStdString(order.email));

        for (const auto& product : order.products) {
            orderDetails += QString("    Code: %1\n    Name: %2\n    Category: %3\n    SubCategory: %4\n    Skin Type: %5\n    Range: %6\n    Price: %7\n    Quantity: %8\n\n")
                                .arg(product.code)
                                .arg(QString::fromStdString(product.name))
                                .arg(QString::fromStdString(product.category))
                                .arg(QString::fromStdString(product.subCategory))
                                .arg(QString::fromStdString(product.skinType))
                                .arg(QString::fromStdString(product.range))
                                .arg(product.price)
                                .arg(product.quantity);
        }

        orderDetails += "-------------------------\n";
    }

    textEdit->setText(orderDetails);
    textEdit->setStyleSheet("background-color: #8B0000; color: white; padding: 10px;"); // Dark red background with white text

    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red
    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (currentUser.username.empty()) {
            showMainPage();
        } else if (isCurrentUserStaff) {
            showStaffMenu();
        } else {
            showCustomerMenu();
        }
    });

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    layout->addWidget(backButton);
    QWidget *viewOrdersWidget = new QWidget(this);
    viewOrdersWidget->setLayout(layout);
    viewOrdersWidget->setStyleSheet("background-color: #FFCDD2;"); // Light red background
    setCentralWidget(viewOrdersWidget);
}

void MainWindow::viewCart() {
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    QString cartDetails;

    stack<Product> tempCart = cart;
    while (!tempCart.empty()) {
        Product product = tempCart.top();
        tempCart.pop();

        cartDetails += QString("Code: %1\nName: %2\nCategory: %3\nSubCategory: %4\nSkin Type: %5\nRange: %6\nPrice: %7\nQuantity: %8\n\n")
                           .arg(product.code)
                           .arg(QString::fromStdString(product.name))
                           .arg(QString::fromStdString(product.category))
                           .arg(QString::fromStdString(product.subCategory))
                           .arg(QString::fromStdString(product.skinType))
                           .arg(QString::fromStdString(product.range))
                           .arg(product.price)
                           .arg(product.quantity);
    }

    textEdit->setText(cartDetails);
    textEdit->setStyleSheet("background-color: #8B0000; color: white; padding: 10px;"); // Dark red background with white text

    QPushButton *backButton = new QPushButton("Back", this);
    backButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red
    connect(backButton, &QPushButton::clicked, this, [this]() {
        if (currentUser.username.empty()) {
            showMainPage();
        } else if (isCurrentUserStaff) {
            showStaffMenu();
        } else {
            showCustomerMenu();
        }
    });

    QPushButton *checkoutButton = new QPushButton("Checkout", this);
    checkoutButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red
    connect(checkoutButton, &QPushButton::clicked, this, &MainWindow::checkout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textEdit);
    layout->addWidget(checkoutButton);
    layout->addWidget(backButton);
    QWidget *cartWidget = new QWidget(this);
    cartWidget->setLayout(layout);
    cartWidget->setStyleSheet("background-color: #FFCDD2;"); // Light red background
    setCentralWidget(cartWidget);
}

void MainWindow::checkout() {
    if (currentUser.username.empty()) {
        QMessageBox::information(this, tr("Checkout"), tr("Please log in or register to proceed with checkout."));
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
        showLoginScreen();
        return;
    }

    double total = 0.0;
    stack<Product> tempCart = cart;

    while (!tempCart.empty()) {
        const Product& product = tempCart.top();
        total += product.price * product.quantity;
        tempCart.pop();
    }

    bool ok;
    QString customerName = QInputDialog::getText(this, tr("Checkout"), tr("Name:"), QLineEdit::Normal, "", &ok);
    if (!ok || customerName.isEmpty()) return;

    QString address = QInputDialog::getText(this, tr("Checkout"), tr("Address:"), QLineEdit::Normal, "", &ok);
    if (!ok || address.isEmpty()) return;

    QString contact = QInputDialog::getText(this, tr("Checkout"), tr("Contact:"), QLineEdit::Normal, "", &ok);
    if (!ok || contact.isEmpty()) return;

    QString email = QInputDialog::getText(this, tr("Checkout"), tr("Email:"), QLineEdit::Normal, "", &ok);
    if (!ok || email.isEmpty()) return;

    vector<Product> productsInCart;
    tempCart = cart;
    while (!tempCart.empty()) {
        productsInCart.push_back(tempCart.top());
        tempCart.pop();
    }

    Order order(customerName.toStdString(), address.toStdString(), contact.toStdString(), email.toStdString(), productsInCart);
    orders.enqueue(order);
    saveOrderToFile(order);

    cart = stack<Product>();
    QMessageBox::information(this, tr("Checkout"), tr("Order placed successfully! Total: %1").arg(total));
    QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
    if (infoBox) {
        infoBox->setStyleSheet("background-color: #FFCDD2;");
    }
    showCustomerMenu();
}

void MainWindow::showStaffMenu() {
    QLabel *staffMenuLabel = new QLabel("Staff Menu", this);
    QFont font = staffMenuLabel->font();
    font.setPointSize(16);
    font.setBold(true);
    staffMenuLabel->setFont(font);
    staffMenuLabel->setAlignment(Qt::AlignCenter);
    staffMenuLabel->setStyleSheet("color: #B22222;"); // Set the title color to red

    QPushButton *addProductButton = new QPushButton("Add Product", this);
    QPushButton *editProductQuantityButton = new QPushButton("Edit Product Quantity", this);
    QPushButton *deleteProductButton = new QPushButton("Delete Product", this);
    QPushButton *displayProductsButton = new QPushButton("Display Products", this);
    QPushButton *searchProductsButton = new QPushButton("Search Products", this);
    QPushButton *viewOrdersButton = new QPushButton("View Orders", this);
    QPushButton *logoutButton = new QPushButton("Logout", this);

    QString buttonStyle = "background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"; // Lighter red
    addProductButton->setStyleSheet(buttonStyle);
    editProductQuantityButton->setStyleSheet(buttonStyle);
    deleteProductButton->setStyleSheet(buttonStyle);
    displayProductsButton->setStyleSheet(buttonStyle);
    searchProductsButton->setStyleSheet(buttonStyle);
    viewOrdersButton->setStyleSheet(buttonStyle);
    logoutButton->setStyleSheet(buttonStyle);

    connect(addProductButton, &QPushButton::clicked, this, &MainWindow::on_addProductButton_clicked);
    connect(editProductQuantityButton, &QPushButton::clicked, this, &MainWindow::on_editProductQuantityButton_clicked);
    connect(deleteProductButton, &QPushButton::clicked, this, &MainWindow::on_deleteProductButton_clicked);
    connect(displayProductsButton, &QPushButton::clicked, this, &MainWindow::on_displayProductsButton_clicked);
    connect(searchProductsButton, &QPushButton::clicked, this, &MainWindow::on_searchProductsButton_clicked);
    connect(viewOrdersButton, &QPushButton::clicked, this, &MainWindow::on_viewOrdersButton_clicked);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::on_logoutButton_clicked);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(staffMenuLabel);
    layout->addWidget(addProductButton);
    layout->addWidget(editProductQuantityButton);
    layout->addWidget(deleteProductButton);
    layout->addWidget(displayProductsButton);
    layout->addWidget(searchProductsButton);
    layout->addWidget(viewOrdersButton);
    layout->addWidget(logoutButton);

    QWidget *staffWidget = new QWidget(this);
    staffWidget->setLayout(layout);
    staffWidget->setStyleSheet("background-color: #FFCDD2;"); // Light red background
    setCentralWidget(staffWidget);
}
void MainWindow::showCustomerMenu() {
    QLabel *customerMenuLabel = new QLabel("Customer Menu", this);
    QFont font = customerMenuLabel->font();
    font.setPointSize(16);
    font.setBold(true);
    customerMenuLabel->setFont(font);
    customerMenuLabel->setAlignment(Qt::AlignCenter);
    customerMenuLabel->setStyleSheet("color: #B22222;"); // Set the title color to red

    QPushButton *displayProductsButton = new QPushButton("Display Products", this);
    QPushButton *searchProductsButton = new QPushButton("Search Products", this);
    QPushButton *viewCartButton = new QPushButton("View Cart", this);
    QPushButton *identifySkinTypeButton = new QPushButton("Identify Skin Type", this);
    QPushButton *logoutButton = new QPushButton("Logout", this);

    QString buttonStyle = "background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"; // Lighter red
    displayProductsButton->setStyleSheet(buttonStyle);
    searchProductsButton->setStyleSheet(buttonStyle);
    viewCartButton->setStyleSheet(buttonStyle);
    identifySkinTypeButton->setStyleSheet(buttonStyle);
    logoutButton->setStyleSheet(buttonStyle);

    connect(displayProductsButton, &QPushButton::clicked, this, &MainWindow::on_displayProductsButton_clicked);
    connect(searchProductsButton, &QPushButton::clicked, this, &MainWindow::on_searchProductsButton_clicked);
    connect(viewCartButton, &QPushButton::clicked, this, &MainWindow::on_viewCartButton_clicked);
    connect(identifySkinTypeButton, &QPushButton::clicked, this, &MainWindow::on_identifySkinTypeButton_clicked);
    connect(logoutButton, &QPushButton::clicked, this, &MainWindow::on_logoutButton_clicked);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(customerMenuLabel);
    layout->addWidget(displayProductsButton);
    layout->addWidget(searchProductsButton);
    layout->addWidget(viewCartButton);
    layout->addWidget(identifySkinTypeButton);
    layout->addWidget(logoutButton);

    QWidget *customerWidget = new QWidget(this);
    customerWidget->setLayout(layout);
    customerWidget->setStyleSheet("background-color: #FFCDD2;"); // Light red background
    setCentralWidget(customerWidget);
}

void MainWindow::addToCartFromDisplay() {
    bool ok;
    int code = QInputDialog::getInt(this, tr("Add to Cart"), tr("Enter Product Code:"), 0, 0, 10000, 1, &ok);
    if (!ok) return;

    Product* product = products.findProduct(code);
    if (product) {
        int quantity = QInputDialog::getInt(this, tr("Add to Cart"), tr("Enter Quantity:"), 1, 1, product->quantity, 1, &ok);
        if (!ok) return;

        Product cartProduct = *product;
        cartProduct.quantity = quantity;
        cart.push(cartProduct);

        QMessageBox::information(this, tr("Add to Cart"), tr("Product added to cart successfully!"));
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
    } else {
        QMessageBox::warning(this, tr("Add to Cart"), tr("Product not found!"));
        QMessageBox *warningBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }
}

void MainWindow::editProductFromDisplay() {
    bool ok;
    int code = QInputDialog::getInt(this, tr("Edit Product"), tr("Enter Product Code:"), 0, 0, 10000, 1, &ok);
    if (!ok) return;

    Product* product = products.findProduct(code);
    if (product) {
        QString name = QInputDialog::getText(this, tr("Edit Product"), tr("Product Name:"), QLineEdit::Normal, QString::fromStdString(product->name), &ok);
        if (!ok || name.isEmpty()) return;

        QString category = QInputDialog::getItem(this, tr("Edit Product"), tr("Category:"), {"Skincare", "Haircare", "Makeup"}, 0, false, &ok);
        if (!ok || category.isEmpty()) return;

        QStringList subCategories;
        if (category == "Skincare") {
            subCategories << "Cleansers" << "Exfoliants" << "Toners" << "Serums" << "Moisturizers" << "Sunscreens" << "Eye Creams" << "Face Masks" << "Spot Treatments" << "Facial Oils" << "Essences" << "Face Mists" << "Lip Care" << "Anti-Aging Products" << "Acne Treatments" << "Brightening Products";
        } else if (category == "Haircare") {
            subCategories << "Shampoo" << "Conditioner" << "Hair Oil" << "Hair Mask" << "Hair Serum" << "Hair Spray" << "Hair Mousse" << "Hair Gel" << "Leave-In Conditioner" << "Hair Cream" << "Hair Wax" << "Hair Foam" << "Hair Balm" << "Hair Treatment" << "Dry Shampoo" << "Heat Protectant" << "Hair Toner" << "Hair Detangler" << "Scalp Scrub" << "Hair Fragrance";
        } else if (category == "Makeup") {
            subCategories << "Foundation" << "Concealer" << "Powder" << "Blush" << "Bronzer" << "Highlighter" << "Contour" << "Primer" << "Eyeshadow" << "Eyeliner" << "Mascara" << "Eyebrow Pencil" << "Eyebrow Gel" << "Lipstick" << "Lip Gloss" << "Lip Liner" << "Lip Balm" << "Setting Spray" << "Setting Powder" << "BB Cream" << "CC Cream" << "Tinted Moisturizer" << "Eyelash Curler" << "Face Mist" << "Makeup Remover" << "Eyebrow Powder" << "Lip Stain" << "Eyeshadow Primer" << "Lip Plumper" << "Color Corrector";
        }

        QString subCategory = QInputDialog::getItem(this, tr("Edit Product"), tr("SubCategory:"), subCategories, 0, false, &ok);
        if (!ok || subCategory.isEmpty()) return;

        QString skinType = QInputDialog::getItem(this, tr("Edit Product"), tr("Skin Type:"), {"Oily", "Dry", "Combination", "Sensitive"}, 0, false, &ok);
        if (!ok || skinType.isEmpty()) return;

        QString range = QInputDialog::getItem(this, tr("Edit Product"), tr("Price Range:"), {"Low", "Medium", "High"}, 0, false, &ok);
        if (!ok || range.isEmpty()) return;

        double price = QInputDialog::getDouble(this, tr("Edit Product"), tr("Price:"), product->price, 0, 10000, 2, &ok);
        if (!ok) return;

        int quantity = QInputDialog::getInt(this, tr("Edit Product"), tr("Quantity:"), product->quantity, 0, 1000, 1, &ok);
        if (!ok) return;

        product->name = name.toStdString();
        product->category = category.toStdString();
        product->subCategory = subCategory.toStdString();
        product->skinType = skinType.toStdString();
        product->range = range.toStdString();
        product->price = price;
        product->quantity = quantity;
        saveProductsToFile();

        QMessageBox::information(this, tr("Edit Product"), tr("Product edited successfully!"));
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
    } else {
        QMessageBox::warning(this, tr("Edit Product"), tr("Product not found!"));
        QMessageBox *warningBox= dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (warningBox) {
            warningBox->setStyleSheet("background-color: #FFCDD2;");
        }
    }
}

void MainWindow::showLoginScreen() {
    QPushButton *registerButton = new QPushButton("Register", this);
    QPushButton *loginButton = new QPushButton("Login", this);
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::on_registerButton_clicked);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::on_loginButton_clicked);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(registerButton);
    layout->addWidget(loginButton);
    QWidget *loginWidget = new QWidget(this);
    loginWidget->setLayout(layout);
    setCentralWidget(loginWidget);
}
void MainWindow::showMainPage() {
    // Set the main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Set the central widget's background color to light red
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #FFCDD2;");
    setCentralWidget(centralWidget);




    QLabel *welcomeLabel = new QLabel("   WELCOME TO COSMOCONTROL", this);
    QFont welcomeFont = welcomeLabel->font();
    welcomeFont.setPointSize(40);
    welcomeFont.setFamily("Roboto");
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #B22222;"); // Set the text color to reddish

    QPushButton *registerButton = new QPushButton("Register", this);
    QPushButton *loginButton = new QPushButton("Login", this);
    QFont buttonFont = registerButton->font();
    buttonFont.setPointSize(14);
    buttonFont.setFamily("Roboto");
    registerButton->setFont(buttonFont);
    loginButton->setFont(buttonFont);
    registerButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 10px; margin-left: 10px;");
    loginButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 10px; margin-left: 10px;");
    connect(registerButton, &QPushButton::clicked, this, &MainWindow::on_registerButton_clicked);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::on_loginButton_clicked);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addStretch();
    headerLayout->addWidget(welcomeLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(registerButton);
    headerLayout->addWidget(loginButton);
    headerLayout->setAlignment(Qt::AlignTop);

    mainLayout->addLayout(headerLayout);

    // Add the buttons in a horizontal layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    QPushButton *displayProductsButton = new QPushButton("Display Products", this);
    QPushButton *searchProductsButton = new QPushButton("Search Products", this);
    QPushButton *viewCartButton = new QPushButton("View Cart", this);
    QPushButton *identifySkinTypeButton = new QPushButton("Identify Skin Type", this);

    displayProductsButton->setFont(buttonFont);
    searchProductsButton->setFont(buttonFont);
    viewCartButton->setFont(buttonFont);
    identifySkinTypeButton->setFont(buttonFont);

    displayProductsButton->setStyleSheet("background-color: #CD5C5C; color: white; border-radius: 15px; padding: 15px;");
    searchProductsButton->setStyleSheet("background-color: #CD5C5C; color: white; border-radius: 15px; padding: 15px;");
    viewCartButton->setStyleSheet("background-color: #CD5C5C; color: white; border-radius: 15px; padding: 15px;");
    identifySkinTypeButton->setStyleSheet("background-color: #CD5C5C; color: white; border-radius: 15px; padding: 15px;");

    connect(displayProductsButton, &QPushButton::clicked, this, &MainWindow::on_displayProductsButton_clicked);
    connect(searchProductsButton, &QPushButton::clicked, this, &MainWindow::on_searchProductsButton_clicked);
    connect(viewCartButton, &QPushButton::clicked, this, &MainWindow::on_viewCartButton_clicked);
    connect(identifySkinTypeButton, &QPushButton::clicked, this, &MainWindow::on_identifySkinTypeButton_clicked);

    buttonLayout->addWidget(displayProductsButton);
    buttonLayout->addWidget(searchProductsButton);
    buttonLayout->addWidget(viewCartButton);
    buttonLayout->addWidget(identifySkinTypeButton);
    buttonLayout->setAlignment(Qt::AlignJustify);

    mainLayout->addLayout(buttonLayout);

    // Add the image
    QLabel *imageLabel = new QLabel(this);
    QPixmap pixmap("C:/Desktop/dsa1/Main image.jpeg"); // Using the new image
    imageLabel->setPixmap(pixmap.scaled(800,600, Qt::KeepAspectRatio)); // Increase image size
    imageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(imageLabel);

    centralWidget->setLayout(mainLayout);
}

void MainWindow::addToCart(const Product& product) {
    cart.push(product);
}

// ProductBST implementation
ProductNode* ProductBST::insert(ProductNode* node, const Product& product) {
    if (!node) return new ProductNode(product);
    if (product.code < node->product.code)
        node->left = insert(node->left, product);
    else
        node->right = insert(node->right, product);
    return node;
}

Product* ProductBST::search(ProductNode* node, int code) {
    if (!node) return nullptr;
    if (node->product.code == code) return &(node->product);
    if (code < node->product.code)
        return search(node->left, code);
    else
        return search(node->right, code);
}

ProductNode* ProductBST::remove(ProductNode* node, int code) {
    if (!node) return nullptr;
    if (code < node->product.code) {
        node->left = remove(node->left, code);
    } else if (code > node->product.code) {
        node->right = remove(node->right, code);
    } else {
        if (!node->left) {
            ProductNode* temp = node->right;
            delete node;
            return temp;
        } else if (!node->right) {
            ProductNode* temp = node->left;
            delete node;
            return temp;
        }
        ProductNode* temp = minValueNode(node->right);
        node->product = temp->product;
        node->right = remove(node->right, temp->product.code);
    }
    return node;
}

ProductNode* ProductBST::minValueNode(ProductNode* node) {
    ProductNode* current = node;
    while (current && current->left != nullptr)
        current = current->left;
    return current;
}
void ProductBST::displayFilteredProducts(QVBoxLayout* layout, QWidget* parent, const string& category, const string& subCategory, const string& skinType, const string& range) {
    inOrderFiltered(root, layout, parent, category, subCategory, skinType, range);
}

void ProductBST::inOrderFiltered(ProductNode* node, QVBoxLayout* layout, QWidget* parent, const string& category, const string& subCategory, const string& skinType, const string& range) {
    if (!node) return;
    inOrderFiltered(node->left, layout, parent, category, subCategory, skinType, range);

    if (node->product.category == category && node->product.subCategory == subCategory && node->product.skinType == skinType && node->product.range == range) {
        // Create a horizontal layout for each product
        QHBoxLayout *productLayout = new QHBoxLayout;

        QLabel *productLabel = new QLabel(QString::fromStdString(
            to_string(node->product.code) + "\t" +
            node->product.name + "\t" +
            node->product.skinType + "\t" +
            to_string(node->product.price) + "\t" +
            to_string(node->product.quantity)));

        productLayout->addWidget(productLabel);

        QSpinBox *quantitySpinBox = new QSpinBox(parent);
        quantitySpinBox->setRange(1, node->product.quantity);
        productLayout->addWidget(quantitySpinBox);

        QPushButton *addButton = new QPushButton("+", parent);
        addButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red
        productLayout->addWidget(addButton);

        QObject::connect(addButton, &QPushButton::clicked, parent, [parent, node, quantitySpinBox]() {
            int quantity = quantitySpinBox->value();
            Product cartProduct = node->product;
            cartProduct.quantity = quantity;
            MainWindow *mainWindow = qobject_cast<MainWindow*>(parent);
            mainWindow->addToCart(cartProduct);
            QMessageBox::information(parent, "Add to Cart", "Product added to cart successfully!");
            QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
            if (infoBox) {
                infoBox->setStyleSheet("background-color: #FFCDD2;");
            }
        });

        layout->addLayout(productLayout);
    }

    inOrderFiltered(node->right, layout, parent, category, subCategory, skinType, range);
}

void ProductBST::inOrder(ProductNode* node, QVBoxLayout* layout, QWidget* parent) {
    if (!node) return;
    inOrder(node->left, layout, parent);

    // Create a horizontal layout for each product
    QHBoxLayout *productLayout = new QHBoxLayout;

    QLabel *productLabel = new QLabel(QString::fromStdString(
        to_string(node->product.code) + "\t" +
        node->product.name + "\t" +
        node->product.skinType + "\t" +
        to_string(node->product.price) + "\t" +
        to_string(node->product.quantity)));

    productLayout->addWidget(productLabel);

    QSpinBox *quantitySpinBox = new QSpinBox(parent);
    quantitySpinBox->setRange(1, node->product.quantity);
    productLayout->addWidget(quantitySpinBox);

    QPushButton *addButton = new QPushButton("+", parent);
    addButton->setStyleSheet("background-color: #CD5C5C; color: white; font-weight: bold; border-radius: 15px; padding: 15px;"); // Lighter red
    productLayout->addWidget(addButton);

    QObject::connect(addButton, &QPushButton::clicked, parent, [parent, node, quantitySpinBox]() {
        int quantity = quantitySpinBox->value();
        Product cartProduct = node->product;
        cartProduct.quantity = quantity;
        MainWindow *mainWindow = qobject_cast<MainWindow*>(parent);
        mainWindow->addToCart(cartProduct);
        QMessageBox::information(parent, "Add to Cart", "Product added to cart successfully!");
        QMessageBox *infoBox = dynamic_cast<QMessageBox *>(QApplication::activeWindow());
        if (infoBox) {
            infoBox->setStyleSheet("background-color: #FFCDD2;");
        }
    });

    layout->addLayout(productLayout);

    inOrder(node->right, layout, parent);
}



void ProductBST::clear(ProductNode* node) {
    if (!node) return;
    clear(node->left);
    clear(node->right);
    delete node;
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
