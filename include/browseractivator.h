#ifndef BROWSERACTIVATOR_H
#define BROWSERACTIVATOR_H

#include <QObject>
#include <QString>

class BrowserActivator : public QObject
{
    Q_OBJECT
public:
    static void openAndActivate(const QString& url);

private:
#ifdef Q_OS_WINDOWS
    static void activateWindowsBrowser(const QString& url);
#elif defined(Q_OS_LINUX)
    static void activateLinuxBrowser(const QString& url);
#elif defined(Q_OS_MACOS)
    static void activateMacBrowser(const QString& url);
#endif
};

#endif
