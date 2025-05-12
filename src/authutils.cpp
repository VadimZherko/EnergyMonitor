#include "../include/authutils.h"
#include <QCryptographicHash>

QString hashPassword(const QString& password, const QString& salt)
{
    QByteArray data = (password + salt).toUtf8();
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}
