cmake_minimum_required(VERSION 3.6...3.14 FATAL_ERROR)
cmake_policy(VERSION 3.6...3.14)

# 从 version.hpp 中获取版本信息
file(READ "include/azh/version.hpp" AZH_VERSION_FILE)
string(REGEX MATCH "#define[ ]+AZH_VERSION_MAJOR[ ]+([0-9]+)" _ ${AZH_VERSION_FILE})
set(AZH_VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "#define[ ]+AZH_VERSION_MINOR[ ]+([0-9]+)" _ ${AZH_VERSION_FILE})
set(AZH_VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "#define[ ]+AZH_VERSION_PATCH[ ]+([0-9]+)" _ ${AZH_VERSION_FILE})
set(AZH_VERSION_PATCH "${CMAKE_MATCH_1}")
string(REGEX MATCH "#define[ ]+AZH_VERSION_REVISION[ ]+([0-9]+)" _ ${AZH_VERSION_FILE})
set(AZH_VERSION_REVISION "${CMAKE_MATCH_1}")

set(AZH_VERSION ${AZH_VERSION_MAJOR}.${AZH_VERSION_MINOR}.${AZH_VERSION_PATCH})

# 设置项目版本
project(VSATEditor VERSION ${AZH_VERSION} LANGUAGES CXX)
message(STATUS "VSATEditor : version " ${AZH_VERSION})

set(ZLIB_INCLUDE_DIR ${CMAKE_PREFIX_PATH}/include)
set(ZLIB_LIBRARY ${CMAKE_PREFIX_PATH}/lib/zlib.lib)

set(CMAKE_CXX_STANDARD 20)

# add_definitions("-DUNICODE" "-D_UNICODE")
# Specify MSVC UTF-8 encoding
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

find_package(spdlog REQUIRED)
find_package(pugixml REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

file(GLOB_RECURSE incs ${CMAKE_SOURCE_DIR}/include/*.h*)
file(GLOB_RECURSE srcs ${CMAKE_SOURCE_DIR}/src/*.c*)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
)

add_library(ExternalLib ${incs} ${srcs})
target_link_libraries(ExternalLib PUBLIC
    spdlog::spdlog
    pugixml::pugixml
    Boost::program_options
)

target_include_directories(ExternalLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(
	DIRECTORY include
	DESTINATION "."
    # PATTERN "CMakeLists.txt" EXCLUDE    
)

install(TARGETS ExternalLib
    EXPORT ExternalLib-targets
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
install(EXPORT ExternalLib-targets
    NAMESPACE ExternalLib::
    FILE ExternalLib-Target.cmake
    DESTINATION lib/cmake/ExternalLib)

include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/ExternalLib-Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ExternalLib-Config.cmake
        INSTALL_DESTINATION lib/cmake
        )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ExternalLib-Config.cmake DESTINATION lib/cmake/ExternalLib)

add_executable(VSATEditor main.cpp)
target_link_libraries(VSATEditor PRIVATE ExternalLib)

add_subdirectory(ui)