#include "../include/mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(int userId, QWidget *parent)
    : QMainWindow(parent), userId(userId)
{
    setWindowTitle("ЭнергоМонитор");
    setFixedSize(1024, 768);

    setStyleSheet(R"(
        QMainWindow { background: #f0f4f7; }
        QLabel { font-size: 16px; color: #2c3e50; }
        QComboBox, QTextEdit {
            background: white;
            border: 2px solid #3498db;
            border-radius: 5px;
            padding: 5px;
        }
        QPushButton {
            background: #3498db;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
        }
    )");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView, 2);

    QWidget *sidebar = new QWidget();
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebar->setFixedWidth(300);

    periodComboBox = new QComboBox();
    periodComboBox->addItems({"Текущий месяц", "Последние 6 месяцев", "Май 2024", "Апрель 2024"});
    refreshButton = new QPushButton("Обновить");

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(periodComboBox);
    controlsLayout->addWidget(refreshButton);

    consumptionLabel = new QLabel("Потребление: ...");
    costLabel = new QLabel("Стоимость: ...");
    docButton = new QPushButton("Нормативные документы");
    tipsArea = new QTextEdit();
    tipsArea->setReadOnly(true);

    sidebarLayout->addLayout(controlsLayout);
    sidebarLayout->addWidget(consumptionLabel);
    sidebarLayout->addWidget(costLabel);
    sidebarLayout->addWidget(docButton);
    sidebarLayout->addWidget(new QLabel("Советы:"));
    sidebarLayout->addWidget(tipsArea);
    mainLayout->addWidget(sidebar);

    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::onPeriodChanged);
    connect(docButton, &QPushButton::clicked, []() {
        BrowserActivator::openAndActivate("https://www.energosbyt.by/ru/info-potrebitelyam/ur-l/tarify/tarify-elektro");
    });

    onPeriodChanged();
    updateAvailableMonths();
}

void MainWindow::loadData(const QString &period)
{
    QSqlQuery query;
    double consumption = 0.0;
    QString queryStr;
    QStringList bindValues;

    if (period == "Текущий месяц")
    {
        QString currentMonth = QDate::currentDate().toString("yyyy-MM");
        queryStr = "SELECT SUM(value) FROM energy_usage "
                   "WHERE user_id = ? AND strftime('%Y-%m', timestamp) = ?";
        bindValues << QString::number(userId) << currentMonth;
    }
    else if (period == "Последние 6 месяцев")
    {
        queryStr = "SELECT SUM(value) FROM energy_usage "
                   "WHERE user_id = ? AND timestamp >= date('now', '-6 months')";
        bindValues << QString::number(userId);
    }
    else if (period.contains(" "))
    {
        QDate monthDate = QDate::fromString(period, "MMMM yyyy");
        queryStr = "SELECT SUM(value) FROM energy_usage "
                   "WHERE user_id = ? AND strftime('%Y-%m', timestamp) = ?";
        bindValues << QString::number(userId) << monthDate.toString("yyyy-MM");
    }

    query.prepare(queryStr);
    for (const QString &val : bindValues)
    {
        query.addBindValue(val);
    }

    if (query.exec() && query.next())
    {
        consumption = query.value(0).toDouble();
    }

    consumptionLabel->setText(QString("Потребление: %1 кВт·ч").arg(consumption));
    costLabel->setText(QString("Стоимость: %1 руб").arg(consumption * 5.0));
}

void MainWindow::createCharts(const QString &period)
{
    QChart *chart = new QChart();
    QLineSeries *series = new QLineSeries();
    QSqlQuery query;
    QString queryStr;
    QStringList bindValues;

    if (period == "Последние 6 месяцев")
    {
        queryStr =
            "SELECT strftime('%Y-%m-01', timestamp) AS month, SUM(value) "
            "FROM energy_usage "
            "WHERE user_id = ? AND timestamp >= date('now', '-6 months') "
            "GROUP BY strftime('%Y-%m', timestamp) "
            "ORDER BY timestamp";
        bindValues << QString::number(userId);
    }
    else if (period == "Текущий месяц")
    {
        QString currentMonth = QDate::currentDate().toString("yyyy-MM");
        queryStr = "SELECT timestamp, value FROM energy_usage "
                   "WHERE user_id = ? AND strftime('%Y-%m', timestamp) = ? "
                   "ORDER BY timestamp";
        bindValues << QString::number(userId) << currentMonth;
    }
    else if (period.contains(" "))
    {
        QDate monthDate = QDate::fromString(period, "MMMM yyyy");
        queryStr = "SELECT timestamp, value FROM energy_usage "
                   "WHERE user_id = ? AND strftime('%Y-%m', timestamp) = ? "
                   "ORDER BY timestamp";
        bindValues << QString::number(userId) << monthDate.toString("yyyy-MM");
    }

    query.prepare(queryStr);
    for (const QString &val : bindValues)
    {
        query.addBindValue(val);
    }

    if (query.exec())
    {
        while (query.next())
        {
            if (period == "Последние 6 месяцев")
            {
                QDateTime monthStart = QDateTime::fromString(query.value(0).toString(), "yyyy-MM-dd");
                series->append(monthStart.toMSecsSinceEpoch(), query.value(1).toDouble());
            } else
            {
                QDateTime timestamp = query.value(0).toDateTime();
                series->append(timestamp.toMSecsSinceEpoch(), query.value(1).toDouble());
            }
        }
    }

    chart->addSeries(series);
    chart->setTitle("Потребление за " + period);
    chart->legend()->hide();

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat(period.contains("месяц") ? "dd.MM" : "MMM yyyy");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("кВт·ч");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView->setChart(chart);
}

void MainWindow::updateTips(const QString &period)
{
    QString tips = "💡 Советы:\n\n";
    QSqlQuery query;

    query.prepare(
        "SELECT text FROM tips "
        "WHERE category IN ('general', :season) "
        "ORDER BY RANDOM() LIMIT 5"
        );

    QString season = "general";
    int month = QDate::currentDate().month();
    if (month >= 11 || month <= 2) season = "winter";
    else if (month >= 6 && month <= 8) season = "summer";

    query.bindValue(":season", season);

    if (query.exec())
    {
        while (query.next())
        {
            tips += "• " + query.value(0).toString() + "\n";
        }
    } else
    {
        tips = "⚠️ Не удалось загрузить советы";
    }

    tipsArea->setText(tips);
}

void MainWindow::updateAvailableMonths()
{
    periodComboBox->clear();
    periodComboBox->addItem("Текущий месяц");
    periodComboBox->addItem("Последние 6 месяцев");

    QSqlQuery query("SELECT DISTINCT strftime('%Y-%m', timestamp) "
                    "FROM energy_usage ORDER BY timestamp DESC");
    while (query.next())
    {
        QString month = query.value(0).toString();
        periodComboBox->addItem(QDate::fromString(month + "-01", "yyyy-MM-dd").toString("MMMM yyyy"));
    }
}

void MainWindow::onPeriodChanged()
{
    QString period = periodComboBox->currentText();
    loadData(period);
    createCharts(period);
    updateTips(period);
}
