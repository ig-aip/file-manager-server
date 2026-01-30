#ifndef LOGGER_H
#define LOGGER_H
#include "QString"
#include <string>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <mutex>

class Logger
{
    const QString filePath{"errorLog.txt"};
    QFile file{filePath};
    std::mutex mutex;

public:
    Logger();

    void message(const std::string& str);

    template<typename T>
    void message(const QString& str);
};

#endif // LOGGER_H
