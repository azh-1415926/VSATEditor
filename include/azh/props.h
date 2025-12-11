#pragma once

#include <iostream>
#include <pugixml.hpp>
#include <sstream>
#include <filesystem>

#include "attribute_table.h"

enum class props_attr_preset
{
    NONE,
    DEBUG_WIN64_CLCOMPILE,
    DEBUG_WIN64_LINK,
    RELEASE_WIN64_CLCOMPILE,
    RELEASE_WIN64_LINK,

    DEBUG_WIN32_CLCOMPILE,
    DEBUG_WIN32_LINK,
    RELEASE_WIN32_CLCOMPILE,
    RELEASE_WIN32_LINK
};

const std::vector<props_attr_preset> default_presets=
{
    props_attr_preset::DEBUG_WIN64_CLCOMPILE,
    props_attr_preset::DEBUG_WIN64_LINK,
    props_attr_preset::RELEASE_WIN64_CLCOMPILE,
    props_attr_preset::RELEASE_WIN64_LINK
};

inline std::pair<std::string, std::string> get_attr_conf_by_preset(const props_attr_preset &preset)
{
    std::pair<std::string, std::string> data;

    std::string condition;
    std::string name;

    switch (preset)
    {
    case props_attr_preset::DEBUG_WIN64_CLCOMPILE:
        condition = "Debug|x64";
        name = "ClCompile";
        break;
    case props_attr_preset::DEBUG_WIN64_LINK:
        condition = "Debug|x64";
        name = "Link";
        break;
    case props_attr_preset::DEBUG_WIN32_CLCOMPILE:
        condition = "Debug|Win32";
        name = "ClCompile";
        break;
    case props_attr_preset::DEBUG_WIN32_LINK:
        condition = "Debug|Win32";
        name = "Link";
        break;

    case props_attr_preset::RELEASE_WIN64_CLCOMPILE:
        condition = "Release|x64";
        name = "ClCompile";
        break;
    case props_attr_preset::RELEASE_WIN64_LINK:
        condition = "Release|x64";
        name = "Link";
        break;
    case props_attr_preset::RELEASE_WIN32_CLCOMPILE:
        condition = "Release|Win32";
        name = "ClCompile";
        break;
    case props_attr_preset::RELEASE_WIN32_LINK:
        condition = "Release|Win32";
        name = "Link";
        break;

    default:
        break;
    }

    data.first = condition;
    data.second = name;

    return data;
}

class props
{
    pugi::xml_document m_doc;
    std::string m_xml_path;

public:
    /* preset 0-> x64 , 1 -> Win32 */
    props(int preset=0);
    explicit props(const std::string &xml_path,int preset=0);

    props(const props &p): m_xml_path(p.m_xml_path)
    {
        std::stringstream ss;
        p.m_doc.save(ss);
        this->m_doc.load(ss);
    }

    props& operator=(const props& p)
    {
        std::stringstream ss;
        p.m_doc.save(ss);
        this->m_doc.load(ss);

        this->m_xml_path=p.m_xml_path;

        return *this;
    }

    void load(const std::string &xml_path)
    {
        m_xml_path=xml_path;
        m_doc.load_file(m_xml_path.c_str());
    }

    void save();
    void save(const std::string &xml_path);

    bool is_load() const { return !m_xml_path.empty()&&std::filesystem::exists(m_xml_path); }
    std::string get_path() { return m_xml_path; }

    bool check();

    /* Property Sheets */
    std::vector<std::string> get_property_sheets();
    int add_property_sheet(const std::string &props_file);
    int remove_property_sheet(const std::string &props_file);

    /* attribute */
    std::string get_attr_by_name(const std::string &name, const props_attr_preset &preset);
    std::string get_attr_by_name(const std::string &name, const std::string& condition, bool isClCompile);
    bool set_attr_by_name(const std::string &name, const std::string &value, const props_attr_preset &preset);
    bool set_attr_by_name(const std::string &name, const std::string &value, const std::string& condition, bool isClCompile);
    bool remove_attr_by_name(const std::string &name, const props_attr_preset &preset);

    bool set_attr(const std::string &name, std::string &value, const props_attr_preset &preset);
    bool remove_attr(const std::string &name, const props_attr_preset &preset);

    /* other */
    std::vector<std::string> get_conditions();

    static props from_project_file(const std::string &project_file, const std::vector<props_attr_preset> &presets=default_presets)
    {
        props source(project_file);
        props dest;

        std::vector<std::string> xpath_strs;

        for (const props_attr_preset &preset : presets)
        {
            const std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);
            xpath_strs.push_back("/Project/ItemDefinitionGroup[@Condition=\"'$(Configuration)|$(Platform)'=='" + conf.first + "'\"]/" + conf.second);
        }

        for (const std::string &xpath : xpath_strs)
        {
            pugi::xpath_node source_node = source.m_doc.select_node(xpath.c_str());
            pugi::xpath_node dest_node = dest.m_doc.select_node(xpath.c_str());

            if (source_node.node() && dest_node.node())
            {
                dest_node.node().remove_children();
                for (const pugi::xml_node &i : source_node.node().children())
                {
                    pugi::xml_node copyed_node = dest_node.node().append_child(i.name());
                    copyed_node.text().set(i.text().get());
                }
            }
        }

        return dest;
    }

private:
    void create(const attribute_table_config &debug_conf, const attribute_table_config &release_conf);
    void create_root();
    bool create_item_group(const attribute_table_config &conf);

public:
    void print_content();
};