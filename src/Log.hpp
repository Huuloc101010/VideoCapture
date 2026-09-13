#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <string>
#include <string_view>
#include <format>
#include <filesystem>
#include <thread>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"

enum class LogType
{
    INFO,
    ERROR,
    WARNING
};

// Log implement
template<typename... Args>
void LogImplement(const LogType& Type, const std::string_view File, const std::string_view Func, const int Line, 
                  std::format_string<Args...> Format, Args&&... Argss)
{
    // use std::format 
    std::string Content = std::format(Format, std::forward<Args>(Argss)...);

    // Cutting file name
    std::string Filename = std::filesystem::path(File).filename().string();

    switch(Type)
    {
        case LogType::INFO:
        {
            std::cout << RESET << "[" << std::this_thread::get_id() << "][" << Filename 
            << ":" << Line << "][" << Func << "] " << Content << std::endl;
            break;
        }
        case LogType::ERROR:
        {
            std::cerr << RED << "[" << std::this_thread::get_id() << "][" << Filename 
            << ":" << Line << "][" << Func << "] " << Content << RESET << std::endl;
            break;
        }
        case LogType::WARNING:
        {
            std::cout << YELLOW << "[" << std::this_thread::get_id() << "][" << Filename 
            << ":" << Line << "][" << Func << "] " << Content << RESET << std::endl;
            break;
        }
    }
}

#define LOGI(fmt, ...) LogImplement(LogType::INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LogImplement(LogType::WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LogImplement(LogType::ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif // LOG_H