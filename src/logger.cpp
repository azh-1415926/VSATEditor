#include <azh/logger.h>

#include <spdlog/spdlog.h>

namespace azh
{
    using namespace spdlog;

    logger::logger(LOGGER_LEVEL level, std::ostream &o) : out(o)
    {
        if (level < GlobalLoggerLevel::getInstance().get())
        {
            shouldLogging = false;
            return;
        }
        shouldLogging = true;

        this->level=level;
    }

    logger::~logger()
    {
        if (!shouldLogging)
        {
            return;
        }

        Logger_Mutex.lock();

        switch (level)
        {
        case LOGGER_LEVEL::LOG_WARNNING:
            spdlog::warn(ss.str());
            break;
        case LOGGER_LEVEL::LOG_INFO:
            spdlog::info(ss.str());
            break;
        case LOGGER_LEVEL::LOG_ERROR:
            spdlog::error(ss.str());
            break;
        case LOGGER_LEVEL::LOG_FATAL:
            spdlog::critical(ss.str());
            break;
        default:
            spdlog::info(ss.str());
            break;
        }

        Logger_Mutex.unlock();
    }
}