#pragma once

#include <cassert>
#include <cstddef>

#define AZH_VERSION_MAJOR 0
#define AZH_VERSION_MINOR 4
#define AZH_VERSION_PATCH 2

#define AZH_VERSION                                                            \
    std::string() + std::to_string(AZH_VERSION_MAJOR) + "." +                  \
        std::to_string(AZH_VERSION_MINOR) + "." +                              \
        std::to_string(AZH_VERSION_PATCH)