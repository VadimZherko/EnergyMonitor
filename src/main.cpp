#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include "../include/loginwindow.h"

void initDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/vadim/repository/EnergyMonitor/db/energy.db");

    if (!db.open())
    {
        qDebug() << "Ошибка подключения к БД:" << db.lastError().text();
        return;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    initDatabase();

    LoginWindow w;
    w.show();

    return a.exec();
}
