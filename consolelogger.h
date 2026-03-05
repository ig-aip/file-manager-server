#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H
#include <mutex>
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>
#include <concepts>
#include <iomanip>
template<typename T>
concept Streamable = requires(std::ostream& os, T value){
    {os << value};
};


class ConsoleLogger
{
    void addToLog(std::ostream& os, const Streamable auto& arg)
    {
        os << arg;
    }
    void addToLog(std::ostream &os, const auto& arg)
    {
        os << " <No operator<< for type: " << typeid(arg).name() << "> ";
    }


    std::mutex mutex;
public:
    template<typename... Args>
    void log(const Args& ... args)
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::stringstream ss;
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        (addToLog(ss,args), ...);
        std::cout << ss.str() << std::endl;
    }


    template<typename... Args>
    void errLog(const Args& ... args)
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::stringstream ss;
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        ss << std::put_time(std::localtime(&time), "[%Y-%m-%d %H:%M:%S] ");
        (addToLog(ss,args), ...);
        std::cerr << ss.str() << "\n";
    }

    ConsoleLogger(){}
};












#endif // CONSOLELOGGER_H
