#pragma once

#include <string>
#include <QString>

/* std::string convert to QString */
inline QString std2qstring(const std::string &s)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    return QString::fromLocal8Bit(s);
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    return QString::fromLocal8Bit(s.data());
#endif
}

/* QString convert to std::string */
inline std::string qstring2std(const QString &s)
{
    return s.toLocal8Bit().toStdString();
}