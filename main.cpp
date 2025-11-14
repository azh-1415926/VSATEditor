#include <azh/logger.h>
#include <azh/version.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <format>
#include <iostream>
#include <stdlib.h>

#include <boost/program_options.hpp>
#include <pugixml.hpp>

#include "azh/props.h"
#include "azh/utils.h"

int main(int argc, char **argv)
{
	// use boost::program_options to parse command line arguments
	// if no arguments, use default values
	// if arguments, use arguments
	// -l or --list : list global attribute table info

	azh::logger::setGlobalLevel(azh::LOGGER_LEVEL::LOG_INFO);

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "print help message")("version,v", "show version")("work_dir,w", boost::program_options::value<std::string>(), "set work directory")("list,l", "list global attribute table info");

	boost::program_options::variables_map vm;
	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);
	}
	catch (const std::exception &e)
	{
		azh::logger(azh::LOGGER_LEVEL::LOG_ERROR) << "Error parsing command line arguments: " << e.what();
		return 1;
	}

	if (vm.count("help"))
	{
		azh::logger(azh::LOGGER_LEVEL::LOG_INFO) << desc << ".";
		return 0;
	}

	if (vm.count("version"))
	{
		azh::logger(azh::LOGGER_LEVEL::LOG_INFO) << "QAESCTools version " << AZH_VERSION << ".\n";
		return 0;
	}

	namespace fs = std::filesystem;

	std::string work_dir = fs::current_path().string();
	if (vm.count("work_dir"))
	{
		work_dir = vm["work_dir"].as<std::string>();

		if (!fs::exists(work_dir))
		{
			azh::logger(azh::LOGGER_LEVEL::LOG_ERROR) << "Work directory " << work_dir << " not exist.";
			return 1;
		}
	}
	// std::replace(work_dir.begin(), work_dir.end(), '\\', '/');
	azh::logger(azh::LOGGER_LEVEL::LOG_INFO) << "Work directory : " << work_dir << ".";

	std::string home_dir = std::getenv("USERPROFILE");
	std::string props_root=home_dir+"\\AppData\\Local\\Microsoft\\MSBuild\\v4.0";
	create_folder(props_root);

	azh::logger(azh::LOGGER_LEVEL::LOG_INFO) << "Global attribute table directory : " << props_root << ".";

	if (vm.count("list"))
	{
		props p(props_root+"\\"+get_user_props_name_by_platform(platform_type::Win64));
		p.print_content();

		return 0;
	}

	return 0;
}
