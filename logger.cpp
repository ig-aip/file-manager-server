#include "logger.h"


Logger::Logger() {
    file.open(QIODevice::ReadWrite);
    if(!file.isOpen()){ throw std::runtime_error("error open logger"); }

}




template<>
void Logger::message<QString>(const QString& str){
    std::lock_guard<std::mutex> lock(mutex);
    QTextStream ss(&file);
    ss << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "] " <<str << '\n';
    ss.flush();
}

void Logger::message(const std::string& str){
    message<QString>(QString::fromStdString(str));
}
