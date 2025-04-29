// loginwindow.cpp

#include "../include/loginwindow.h"
#include "../include/mainwindow.h"
#include "../include/authutils.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSqlQuery>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    setWindowTitle("Вход в систему");
    setFixedSize(400, 300); // Фиксированный размер

    // Стили
    setStyleSheet(R"(
        QWidget {
            background-color: #2c3e50;
            font-family: Arial;
        }
        QLineEdit {
            background: #34495e;
            border: 2px solid #3498db;
            border-radius: 5px;
            padding: 10px;
            color: white;
        }
        QPushButton {
            background: #3498db;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            font-size: 16px;
        }
        QPushButton:hover {
            background: #2980b9;
        }
        QLabel {
            color: white;
            font-size: 14px;
        }
    )");

    // Логотип
    QLabel *logo = new QLabel("ЭнергоМонитор");
    logo->setStyleSheet("font-size: 24px; color: #3498db; font-weight: bold;");
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo);
    houseInput = new QLineEdit;
    apartmentInput = new QLineEdit;
    passwordInput = new QLineEdit;
    passwordInput->setEchoMode(QLineEdit::Password);

    layout->addWidget(new QLabel("Номер дома:"));
    layout->addWidget(houseInput);
    layout->addWidget(new QLabel("Номер квартиры:"));
    layout->addWidget(apartmentInput);
    layout->addWidget(new QLabel("Пароль:"));
    layout->addWidget(passwordInput);

    QPushButton *loginButton = new QPushButton("Войти");
    layout->addWidget(loginButton);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::onLoginClicked() {
    int house = houseInput->text().toInt();
    int apartment = apartmentInput->text().toInt();
    QString password = passwordInput->text();

    QSqlQuery query;
    query.prepare("SELECT id, password_hash, salt FROM users "
                  "WHERE house_number = ? AND apartment_number = ?");
    query.addBindValue(house);
    query.addBindValue(apartment);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "Ошибка", "Пользователь не найден");
        return;
    }

    QString storedHash = query.value(1).toString();
    QString salt = query.value(2).toString();
    QString inputHash = hashPassword(password, salt);

    if (inputHash == storedHash) {
        MainWindow *mainWin = new MainWindow(query.value(0).toInt());
        mainWin->show();
        this->close();
    } else {
        QMessageBox::warning(this, "Ошибка", "Неверный пароль");
    }
}
