#pragma once

#include <filesystem>

namespace azh::utils
{
    inline bool create_folder(const std::string &path)
    {
        try
        {
            if (std::filesystem::create_directories(path))
            {
                return true;
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
        }

        return false;
    }
}
