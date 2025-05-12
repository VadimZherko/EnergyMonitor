#include "../include/browseractivator.h"
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QTimer>
#include <QThread>
#include <QDebug>

void BrowserActivator::openAndActivate(const QString& url)
{
#ifdef Q_OS_LINUX
    activateLinuxBrowser(url);
#else
    QDesktopServices::openUrl(QUrl(url));
    QTimer::singleShot(3000, [url]() {
#   ifdef Q_OS_WINDOWS
        activateWindowsBrowser(url);
#   elif defined(Q_OS_MACOS)
        activateMacBrowser(url);
#   endif
    });
#endif
}

// Windows реализация
#ifdef Q_OS_WINDOWS
#include <windows.h>

void BrowserActivator::activateWindowsBrowser(const QString& url)
{
    QProcess::execute("powershell", {
                                        "-Command",
                                        "$process = Get-Process | Where-Object { "
                                        "$_.MainWindowTitle -match 'Chrome|Firefox|Edge|Opera' } | "
                                        "Select-Object -First 1; "
                                        "if ($process) { "
                                        "Add-Type -TypeDefinition '"
                                        "using System; using System.Runtime.InteropServices; "
                                        "public class Win32 { "
                                        "[DllImport(\"user32.dll\")] "
                                        "public static extern bool SetForegroundWindow(IntPtr hWnd); "
                                        "}' | Out-Null; "
                                        "[Win32]::SetForegroundWindow($process.MainWindowHandle) }"
                                        " else { Start-Process " + url + " }"
                                    });
}
#endif

// Linux реализация
#ifdef Q_OS_LINUX
void BrowserActivator::activateLinuxBrowser(const QString& url)
{
    // Проверка запущенных браузеров
    bool isBrowserRunning = false;
    QProcess psProcess;
    psProcess.start("ps", {"-A"});
    if(psProcess.waitForFinished(2000)) {
        QString output = psProcess.readAllStandardOutput();
        isBrowserRunning = output.contains("firefox") ||
                           output.contains("chrome") ||
                           output.contains("chromium");
    }

    // Запуск браузера если не запущен
    if(!isBrowserRunning) {
        QProcess::startDetached("xdg-open", {url});
        QThread::sleep(3);
    }

    // Активация окна
    bool activated = false;
    const QStringList browsers = {"Firefox", "Chrome", "Chromium"};
    foreach(const auto& browser, browsers) {
        QProcess p;
        p.start("wmctrl", {"-a", browser});
        p.waitForFinished(500);
        if(p.exitCode() == 0) {
            activated = true;
            break;
        }
    }

    // Повторный запуск при неудаче
    if(!activated) {
        QProcess::startDetached("xdg-open", {url});
    }
}
#endif

// macOS реализация
#ifdef Q_OS_MACOS
void BrowserActivator::activateMacBrowser(const QString& url)
{
    const QStringList applescripts = {
        "tell application \"Google Chrome\" to activate",
        "tell application \"Safari\" to activate",
        "tell application \"Firefox\" to activate"
    };

    foreach(const auto& script, applescripts) {
        if(QProcess::execute("osascript", {"-e", script}) == 0) {
            return;
        }
    }
    QDesktopServices::openUrl(QUrl(url));
}
#endif
