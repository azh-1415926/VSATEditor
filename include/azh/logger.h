#pragma once

#include <string.h>
#include <iostream>
#include <sstream>
#include <mutex>

#define LOGGER_WARNNING 0
#define LOGGER_INFO 1
#define LOGGER_ERROR 2
#define LOGGER_FATAL 3

namespace azh
{
    enum class LOGGER_LEVEL : int
    {
        LOG_WARNNING = LOGGER_WARNNING,
        LOG_INFO = LOGGER_INFO,
        LOG_ERROR = LOGGER_ERROR,
        LOG_FATAL = LOGGER_FATAL
    };

    class GlobalLoggerLevel
    {
    public:
        static GlobalLoggerLevel &getInstance()
        {
            static GlobalLoggerLevel instance;
            return instance;
        }
        LOGGER_LEVEL get() const { return m_Instance; }
        void set(LOGGER_LEVEL data) { m_Instance = data; }

    private:
        explicit GlobalLoggerLevel() : m_Instance(LOGGER_LEVEL::LOG_INFO) {}
        LOGGER_LEVEL m_Instance;
    };

    class logger
    {
    private:
        bool shouldLogging;
        std::ostream &out;
        std::stringstream ss;
        std::mutex Logger_Mutex;
        LOGGER_LEVEL level;

    public:
        logger(LOGGER_LEVEL level = LOGGER_LEVEL::LOG_INFO, std::ostream &o = std::cout);

        ~logger();

        logger(const logger &l) = delete;
        logger operator=(const logger &l) = delete;

        void *operator new(size_t size) = delete;
        void *operator new[](size_t size) = delete;
        void operator delete(void *ptr) = delete;
        void operator delete[](void *ptr) = delete;

        static void setGlobalLevel(LOGGER_LEVEL level) { GlobalLoggerLevel::getInstance().set(level); }

        template <class T>
        logger &operator<<(const T &type)
        {
            if (!shouldLogging)
            {
                return *this;
            }

            ss << type;

            return *this;
        }

    private:
    };
}