#pragma once

#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>

namespace azh::utils
{
    template <typename _type>
    struct _has_to_string
    {
    private:
        template <typename U>
        static auto test(int) -> decltype(std::declval<U>().toString(), std::true_type{});

        template <typename>
        static std::false_type test(...);

    public:
        static constexpr bool value = decltype(test<_type>(0))::value;
    };

    template <typename T, typename U>
    struct decay_equiv : std::is_same<typename std::decay<T>::type, U>::type
    {
    };

    template <typename _type>
    inline typename std::enable_if<!_has_to_string<_type>::value, std::string>::type
    to_string(const _type &t)
    {
        std::stringstream ss;
        if (decay_equiv<_type, std::string>::value)
        {
            ss << "\"" << t << "\"";
        }
        else if (decay_equiv<_type, char>::value)
        {
            ss << "\'" << t << "\'";
        }
        else
        {
            ss << t;
        }

        return ss.str();
    }

    template <typename _type>
    inline typename std::enable_if<_has_to_string<_type>::value, std::string>::type
    to_string(const _type &t)
    {
        std::ostringstream oss;
        // oss << typeid(t).name();
        oss << t.toString();
        return oss.str();
    }

    inline std::string get_current_time(const std::string &year_month_day = "", const char delimiter = '_', const std::string &hour_minute_second = "")
    {
        time_t now;
        time(&now);
        tm p = *localtime(&now);

        std::string str;
        str += std::to_string(p.tm_year + 1900);

        if (year_month_day.size() > 0)
            str += year_month_day[0];
        str += std::to_string((p.tm_mon + 1) / 10) + std::to_string((p.tm_mon + 1) % 10);
        if (year_month_day.size() > 1)
            str += year_month_day[1];
        str += std::to_string(p.tm_mday / 10) + std::to_string(p.tm_mday % 10);
        str += delimiter;
        str += std::to_string(p.tm_hour / 10) + std::to_string(p.tm_hour % 10);
        if (hour_minute_second.size() > 0)
            str += hour_minute_second[0];
        str += std::to_string(p.tm_min / 10) + std::to_string(p.tm_min % 10);
        if (hour_minute_second.size() > 1)
            str += hour_minute_second[1];
        str += std::to_string(p.tm_sec / 10) + std::to_string(p.tm_sec % 10);

        return str;
    }

    inline bool str_has_same_prefix(const std::string &s1, const std::string &s2, char delimiter)
    {
        int i1 = s1.find_first_of(delimiter);
        int i2 = s2.find_first_of(delimiter);

        if (i1 == std::string::npos || i2 == std::string::npos || i1 != i2)
        {
            return false;
        }

        std::string p1(s1.begin(), s1.begin() + i1);
        std::string p2(s2.begin(), s2.begin() + i2);

        if (p1 != p2)
        {
            return false;
        }

        return true;
    }

#define REGISTER_LOGGER_TYPE(L_TYPE, MSG)                                                                        \
    template <typename T, typename... Args>                                                                      \
    void L_TYPE(const T &t, const Args &...args)                                                                 \
    {                                                                                                            \
        m_logger_mutex_private.lock();                                                                           \
        m_out_private << "[ ";                                                                                   \
        m_out_private << get_current_time("--", ' ', "::");                                                      \
        m_out_private << " ]";                                                                                   \
        m_out_private << MSG;                                                                                    \
        print(t, args...);                                                                                       \
        m_out_private << std::flush;                                                                             \
        m_logger_mutex_private.unlock();                                                                         \
    }                                                                                                            \
    void L_TYPE(const std::vector<std::string> &logs, bool to_print_header = true, bool to_print_newline = true) \
    {                                                                                                            \
        m_logger_mutex_private.lock();                                                                           \
        if (to_print_header)                                                                                     \
        {                                                                                                        \
            m_out_private << "[ ";                                                                               \
            m_out_private << get_current_time("--", ' ', "::");                                                  \
            m_out_private << " ]";                                                                               \
            m_out_private << MSG;                                                                                \
        }                                                                                                        \
        for (auto i : logs)                                                                                      \
        {                                                                                                        \
            m_out_private << i;                                                                                  \
        }                                                                                                        \
        if (to_print_newline)                                                                                    \
        {                                                                                                        \
            m_out_private << "\n";                                                                               \
        }                                                                                                        \
        m_out_private << std::flush;                                                                             \
        m_logger_mutex_private.unlock();                                                                         \
    }

    class _logger
    {
        /* log file stream */
        std::ofstream m_logfile_stream_private;
        /* use to print log */
        std::ostream &m_out_private;
        /* mutex of current logger */
        std::mutex m_logger_mutex_private;
        /* current log file path */
        std::string m_log_filepath_private;
        /* use stdout */
        bool m_used_stdout_private;

    public:
        _logger() : m_out_private(std::cout), m_used_stdout_private(true) {}
        _logger(const std::string &out_file) : m_out_private(m_logfile_stream_private), m_used_stdout_private(false)
        {
            m_logfile_stream_private.open(out_file, std::ios::out);
        }

        _logger(const _logger &l) : m_out_private(l.m_used_stdout_private ? std::cout : m_logfile_stream_private), m_log_filepath_private(l.m_log_filepath_private), m_used_stdout_private(l.m_used_stdout_private)
        {
            if (l.m_logfile_stream_private.is_open())
            {
                m_logfile_stream_private.open(m_log_filepath_private, std::ios::out);
            }
        }

        ~_logger()
        {
            if (m_logfile_stream_private.is_open())
            {
                m_logfile_stream_private.close();
            }
        }

        void open(const std::string &filepath)
        {
            if (m_used_stdout_private)
            {
                this->error("Current logger print to stdout, can not open file : ", filepath);
                return;
            }

            if (str_has_same_prefix(filepath, m_log_filepath_private, '-'))
            {
                return;
            }

            m_log_filepath_private = filepath;

            if (m_logfile_stream_private.is_open())
                m_logfile_stream_private.close();

            m_logfile_stream_private.open(m_log_filepath_private);
        }

        template <typename T, typename... Args>
        void print(const T &t)
        {
            m_out_private << t << "\n";
        }

        template <typename T, typename... Args>
        void print(const T &t, const Args &...args)
        {
            m_out_private << t;
            print(args...);
        }

        _logger &operator=(const _logger &l) = delete;

        template <typename T>
        _logger &operator<<(const T &t)
        {
            m_logger_mutex_private.lock();

            m_out_private << t;
            m_out_private << std::flush;

            m_logger_mutex_private.unlock();

            return *this;
        }

        REGISTER_LOGGER_TYPE(debug, "[ DEBUG   ] ")
        REGISTER_LOGGER_TYPE(info, "[ INFO    ] ")
        REGISTER_LOGGER_TYPE(warning, "[ WARNING ] ")
        REGISTER_LOGGER_TYPE(error, "[ ERROR   ] ")
        REGISTER_LOGGER_TYPE(fatal, "[ FATAL   ] ")
    };

    class logger
    {
    private:
        logger() {}

    public:
        static _logger *getInstance()
        {
            static logger l;

            return l.getLoggerSingleInstance();
        }

        static _logger *getInstance(const std::string &log_name)
        {
            static logger l;
            if (log_name.empty())
            {
                return nullptr;
            }

            return l.getLoggerSingleInstance(log_name);
        }

    private:
        _logger *getLoggerSingleInstance()
        {
            static _logger logger;
            return &logger;
        }

        _logger *getLoggerSingleInstance(const std::string &log_name)
        {
            std::string log_filepath = log_name + "-" + get_current_time() + ".log";
            static _logger logger(log_filepath);
            logger.open(log_filepath);
            return &logger;
        }
    };

#define REGISTER_LOGGER_WRAPPER_PRINT_TYPE(type) \
    logger_wrapper &operator<<(const type &t)    \
    {                                            \
        *m_logger_private << t;                  \
        return *this;                            \
    }

    class logger_wrapper
    {
        _logger *m_logger_private;
        size_t m_print_type_private;
        bool m_to_print_header_private;
        bool m_to_print_newline_private;
        std::vector<std::string> m_contents_private;

    public:
        explicit logger_wrapper(size_t type = 0, const std::string &log_name = "", bool to_print_header = false, bool to_print_newline = true) : m_logger_private(nullptr), m_print_type_private(type), m_to_print_header_private(to_print_header), m_to_print_newline_private(to_print_newline)
        {
            if (log_name.empty())
                m_logger_private = logger::getInstance();
            else
                m_logger_private = logger::getInstance(log_name);
        }
        ~logger_wrapper()
        {
            switch (m_print_type_private)
            {
            case 0:
                m_logger_private->debug(m_contents_private, m_to_print_header_private, m_to_print_newline_private);
                break;

            case 1:
                m_logger_private->info(m_contents_private, m_to_print_header_private, m_to_print_newline_private);
                break;

            case 2:
                m_logger_private->warning(m_contents_private, m_to_print_header_private, m_to_print_newline_private);
                break;

            case 3:
                m_logger_private->error(m_contents_private, m_to_print_header_private, m_to_print_newline_private);
                break;

            case 4:
                m_logger_private->fatal(m_contents_private, m_to_print_header_private, m_to_print_newline_private);
                break;

            default:
                break;
            }
        }

        logger_wrapper(const logger_wrapper &l) = delete;
        logger_wrapper &operator=(const logger_wrapper &l) = delete;

        template <class _type>
        logger_wrapper &operator<<(const _type &t)
        {
            m_contents_private.push_back(to_string(t));
            return *this;
        }
    };
}

using aDebug = azh::utils::logger_wrapper;

#define AZH_DEBUG 0
#define AZH_INFO 1
#define AZH_WARNING 2
#define AZH_ERROR 3
#define AZH_FATAL 4