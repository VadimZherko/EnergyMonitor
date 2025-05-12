#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    LoginWindow(QWidget *parent = nullptr);

private slots:
    void onLoginClicked();

private:
    QLineEdit *houseInput;
    QLineEdit *apartmentInput;
    QLineEdit *passwordInput;
};
