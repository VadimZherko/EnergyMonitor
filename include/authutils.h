// authutils.h
#pragma once
#include <QString>

QString hashPassword(const QString& password, const QString& salt);
