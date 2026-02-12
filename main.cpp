#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "azh/props.h"
#include "azh/utils/file.hpp"
#include "azh/utils/logger.hpp"
#include "azh/version.hpp"

int main(int argc, char **argv)
{
    namespace fs = std::filesystem;

    std::string work_dir = fs::current_path().string();

    if (!fs::exists(work_dir))
    {
        aDebug(AZH_ERROR) << "Work directory " << work_dir << " not exist.";
        return 1;
    }
    // std::replace(work_dir.begin(), work_dir.end(), '\\', '/');
    aDebug(AZH_INFO) << "Work directory : " << work_dir << ".";

    std::string home_dir = std::getenv("USERPROFILE");
    std::string props_root =
        home_dir + "\\AppData\\Local\\Microsoft\\MSBuild\\v4.0";
    azh::utils::create_folder(props_root);

    aDebug(AZH_INFO) << "Global attribute table directory : " << props_root
                     << ".";

    props p(props_root + "\\" +
            get_user_props_name_by_platform(platform_type::Win64));
    p.print_content();

    return 0;
}
