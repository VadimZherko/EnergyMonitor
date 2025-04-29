#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSqlQuery>

//QT_CHARTS_USE_NAMESPACE

    class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(int userId, QWidget *parent = nullptr);

private slots:
    void onPeriodChanged();

private:
    int userId;
    QChartView *chartView;
    QLabel *consumptionLabel;
    QLabel *costLabel;
    QTextEdit *tipsArea;
    QComboBox *periodComboBox;
    QPushButton *refreshButton;

    void loadData(const QString &period);
    void createCharts(const QString &period);
    void updateTips(const QString &period);

    void updateAvailableMonths();
};

#endif // MAINWINDOW_H
