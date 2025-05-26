/*
    基于spdlog进行日志封装
    使用者可以按照如下方式使用：
    1. 自主决定日志等级
    2. 选择文件输出or控制台输出
    3. 使用C++格式化字符串控制日志内容

    输出格式：
    [年-月-日 时:分:秒] [日志等级] [文件名:行号] 日志内容
    例子：
    [2025-04-25 22:25:25] [info] [test.cc:18] hello world
    使用方式：
    LOG(level, format, args)
*/

#ifndef __rpc_log_h__
#define __rpc_log_h__

#include <memory>
#include <string>
#include <mutex>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"    // 文件日志
#include "spdlog/sinks/stdout_color_sinks.h" // 控制台彩色日志

namespace log_system
{
    // 日志等级，与spdlog等级对应
    enum class Level
    {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class LogSystem
    {
    private:
        const std::string filepath = "log/log.txt";
        LogSystem()
        {
            // 默认等级
            spdlog::set_level(spdlog::level::debug);
            // 默认格式
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
            // 默认控制台打印
            logger_ = spdlog::stdout_color_mt("console_log");
        }

        // 禁用拷贝构造和赋值
        LogSystem(const LogSystem &) = delete;
        LogSystem &operator=(const LogSystem &) = delete;

    public:
        static std::shared_ptr<LogSystem> getInstance()
        {
            // if (!baseLog_)
            // {
            //     std::unique_lock<std::mutex> lock(single_mtx_);
            //     if (!baseLog_)
            //         baseLog_ = std::shared_ptr<LogSystem>(new LogSystem());
            // }

            // 代替双检锁
            static std::once_flag init_flag;
            std::call_once(init_flag, []
                           { baseLog_ = std::shared_ptr<LogSystem>(new LogSystem()); });

            return baseLog_;
        }

        // 启用文件输出
        void enableFileLog()
        {
            std::unique_lock<std::mutex> lock(mode_mtx_);
            spdlog::drop_all(); // 清除上一次的指针
            logger_ = spdlog::basic_logger_mt("file_log", filepath);
        }

        // 启用控制台输出
        void enableConsoleLog()
        {
            std::unique_lock<std::mutex> lock(mode_mtx_);
            spdlog::drop_all(); // 清除上一次的指针
            logger_ = spdlog::stdout_color_mt("console_log");
        }

        // 设置日志等级
        void setLevel(const Level l)
        {
            switch (l)
            {
            case Level::Debug:
                spdlog::set_level(spdlog::level::debug);
                break;
            case Level::Info:
                spdlog::set_level(spdlog::level::info);
                break;
            case Level::Warning:
                spdlog::set_level(spdlog::level::warn);
                break;
            case Level::Error:
                spdlog::set_level(spdlog::level::err);
                break;
            case Level::Critical:
                spdlog::set_level(spdlog::level::critical);
                break;
            default:
                break;
            }
        }

        // 获取日志指针
        std::shared_ptr<spdlog::logger> getLogger()
        {
            return logger_;
        }

    private:
        static std::shared_ptr<LogSystem> baseLog_;
        // 控制台指针
        std::shared_ptr<spdlog::logger> logger_;
        // 单例锁
        // static std::mutex single_mtx_;
        // 模式切换锁
        std::mutex mode_mtx_;
    };

    // std::mutex LogSystem::single_mtx_;
    std::shared_ptr<LogSystem> LogSystem::baseLog_ = nullptr;

    // 获取日志系统类对象
    std::shared_ptr<LogSystem> ls = LogSystem::getInstance();

#define ENABLE_FILE_LOG() ls->enableFileLog()
#define ENABLE_CONSOLE_LOG() ls->enableConsoleLog()

// 日志宏定义，带有文件名和行号
// ##运算符的主要作用是处理没有提供可变参数时的逗号问题
#define LOG_DEBUG(format, ...) \
    ls->getLogger()->debug("[{}:{}] " format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    ls->getLogger()->info("[{}:{}] " format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    ls->getLogger()->warn("[{}:{}] " format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    ls->getLogger()->error("[{}:{}] " format, __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_CRITICAL(format, ...) \
    ls->getLogger()->critical("[{}:{}] " format, __FILE__, __LINE__, ##__VA_ARGS__)

// 通用日志宏，可以指定日志级别
#define LOG(level, format, ...)              \
    switch (level)                           \
    {                                        \
    case log_system::Level::Debug:           \
        LOG_DEBUG(format, ##__VA_ARGS__);    \
        break;                               \
    case log_system::Level::Info:            \
        LOG_INFO(format, ##__VA_ARGS__);     \
        break;                               \
    case log_system::Level::Warning:         \
        LOG_WARN(format, ##__VA_ARGS__);     \
        break;                               \
    case log_system::Level::Error:           \
        LOG_ERROR(format, ##__VA_ARGS__);    \
        break;                               \
    case log_system::Level::Critical:        \
        LOG_CRITICAL(format, ##__VA_ARGS__); \
        break;                               \
    default:                                 \
        LOG_INFO(format, ##__VA_ARGS__);     \
        break;                               \
    }

}

#endif