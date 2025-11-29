#pragma once

#include <string>
#include <vector>

enum class configuration_type
{
    Debug = 0,
    Release,
    MinSizeRel,
    RelWithDebInfo
};

enum class platform_type
{
    Win64 = 0,
    Win32
};

/* attribute table */
struct attribute_table_config
{
    /* Debug or Release or ... */
    configuration_type build_type;
    /* x64 or Win32 or ... */
    platform_type platform;

    /* include directories */
    std::vector<std::string> addl_inc_dirs;
    /* definitions */
    std::vector<std::string> preprocessor_defs;
    /* compile option */
    std::vector<std::string> addl_options;

    /* link directories */
    std::vector<std::string> addl_lib_dirs;
    /* link libraries */
    std::vector<std::string> addl_deps;

    attribute_table_config(int preset = 0)
    {
        switch (preset)
        {
        /* Debug x64 */
        case 0:
            build_type = configuration_type::Debug;
            platform = platform_type::Win64;

            preprocessor_defs.push_back("_DEBUG");
            preprocessor_defs.push_back("_CONSOLE");
            break;

        /* Release x64 */
        case 1:
            build_type = configuration_type::Release;
            platform = platform_type::Win64;

            preprocessor_defs.push_back("NDEBUG");
            preprocessor_defs.push_back("_CONSOLE");
            break;

        /* Debug Win32 */
        case 2:
            build_type = configuration_type::Debug;
            platform = platform_type::Win32;

            preprocessor_defs.push_back("_DEBUG");
            preprocessor_defs.push_back("_CONSOLE");
            break;

        /* Release Win32 */
        case 3:
            build_type = configuration_type::Release;
            platform = platform_type::Win32;

            preprocessor_defs.push_back("NDEBUG");
            preprocessor_defs.push_back("_CONSOLE");
            break;

        default:
            break;
        }
    }
};

inline std::string get_condition(const configuration_type& configuration,const platform_type& platform)
{
    std::string condition;

    switch (configuration)
    {
    case configuration_type::Debug:
        condition += "Debug|";
        break;

    case configuration_type::Release:
        condition += "Release|";
        break;

    default:
        return "";
        break;
    }

    switch (platform)
    {
    case platform_type::Win64:
        condition += "x64";
        break;

    case platform_type::Win32:
        condition += "Win32";
        break;

    default:
        return "";
        break;
    }

    return condition;
}

/* combine [1,2,3], to ";1;2;3" */
inline std::string combine_attribute_value(const std::vector<std::string> &values, const std::string &delimiter = ";")
{
    std::string str;

    for (int i = 0; i < values.size(); i++)
    {
        str += delimiter;
        str += values[i];
    }

    return str;
}

inline std::string get_user_props_name_by_platform(const platform_type& platform)
{
    std::string _platform;

    switch (platform)
    {
    case platform_type::Win64:
        _platform = "x64";
        break;

    case platform_type::Win32:
        _platform = "Win32";
        break;

    default:
        return "";
        break;
    }

    return "Microsoft.Cpp."+_platform+".user.props";
}