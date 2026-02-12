#include "azh/props.h"
#include "azh/utils/logger.hpp"

props::props(int preset)
{
    if(preset==0)
    {
        attribute_table_config debug(0);
        attribute_table_config release(1);

        create(debug, release);
    }
    else
    {
        attribute_table_config debug(2);
        attribute_table_config release(3);

        create(debug, release);
    }
}

props::props(const std::string &xml_path, int preset): m_xml_path(xml_path)
{
    pugi::xml_parse_result result = m_doc.load_file(m_xml_path.c_str());

    if (!result)
    {
        aDebug(AZH_WARNING) << "Load xml file error : " << result.description();
        aDebug(AZH_INFO) << "Try to init a empty xml.";

        if(preset==0)
        {
            attribute_table_config debug(0);
            attribute_table_config release(1);

            create(debug, release);
        }
        else
        {
            attribute_table_config debug(2);
            attribute_table_config release(3);

            create(debug, release);
        }
    }
}

void props::save()
{
    if(m_xml_path.empty())
    {
        aDebug(AZH_ERROR) << "Save - empty xml path, not set default save path.";
        return;
    }

    if(!std::filesystem::exists(std::filesystem::path(m_xml_path).parent_path()))
    {
        aDebug(AZH_ERROR) << "Save - invalid save path : "<<m_xml_path;
        return;
    }

    save(m_xml_path);
}

void props::save(const std::string &xml_path)

{
    if (m_doc.save_file(xml_path.c_str()))
    {
        aDebug(AZH_INFO) << "Save to " << std::filesystem::absolute(xml_path) << ".";

        aDebug(AZH_INFO) << "Content:";
        m_doc.save(std::cout);
        this->m_xml_path=xml_path;
    }
    else
    {
        aDebug(AZH_ERROR) << "Save xml file error.";
    }
}

bool props::check()
{
    pugi::xml_node project = m_doc.child("Project");
    if (project.empty())
    {
        aDebug(AZH_ERROR) << "Check - can not find 'Project' node.";
        return false;
    }

    for (pugi::xml_node import_group : project.children("ItemDefinitionGroup"))
    {
        pugi::xml_node attr_of_clcompile = import_group.child("ClCompile");
        pugi::xml_node attr_of_link = import_group.child("Link");

        if (attr_of_clcompile && attr_of_link)
        {
            return true;
        }
    }

    aDebug(AZH_ERROR) << "Check - can not find ClCompile/Link in 'ItemDefinitionGroup'.";

    return false;
}

std::vector<std::string> props::get_property_sheets()
{
    if (!check())
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> property_sheets;

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node import_group : project.children("ImportGroup"))
    {
        pugi::xml_attribute label_attr = import_group.attribute("Label");
        if (label_attr && std::string(label_attr.value()) == "PropertySheets")
        {
            for (pugi::xml_node import_node : import_group.children("Import"))
            {
                pugi::xml_attribute project_attr = import_node.attribute("Project");
                if (project_attr)
                {
                    property_sheets.push_back(project_attr.value());
                }
            }
        }
    }

    return property_sheets;
}

int props::add_property_sheet(const std::string &props_file)
{
    if (!check())
    {
        return -1;
    }

    std::vector<std::string> property_sheets = get_property_sheets();

    for (const auto &sheet : property_sheets)
    {
        if (sheet == props_file)
        {
            return 1;
        }
    }

    pugi::xml_node project = m_doc.child("Project");
    bool property_sheets_found = false;

    for (pugi::xml_node import_group : project.children("ImportGroup"))
    {
        property_sheets_found = true;

        pugi::xml_attribute label_attr = import_group.attribute("Label");
        if (label_attr && std::string(label_attr.value()) == "PropertySheets")
        {
            pugi::xml_node import_node = import_group.append_child("Import");
            import_node.append_attribute("Project") = props_file.c_str();
            return 0;
        }
    }

    if (!property_sheets_found)
    {
        pugi::xml_node import_group = project.append_child("ImportGroup");
        import_group.append_attribute("Label") = "PropertySheets";

        pugi::xml_node importNode = import_group.append_child("Import");
        importNode.append_attribute("Project") = props_file.c_str();

        return 0;
    }

    aDebug(AZH_ERROR) << "Error in add_property_sheet.";

    return -1;
}

int props::remove_property_sheet(const std::string &props_file)
{
    if (!check())
    {
        return -1;
    }

    bool props_found = false;
    std::vector<std::string> property_sheets = get_property_sheets();

    for (const auto &sheet : property_sheets)
    {
        if (sheet == props_file)
        {
            props_found = true;
            break;
        }
    }

    if (!props_found)
    {
        return 1;
    }

    pugi::xml_node project = m_doc.child("Project");
    bool property_sheets_found = false;

    for (pugi::xml_node import_group : project.children("ImportGroup"))
    {
        property_sheets_found = true;

        pugi::xml_attribute label_attr = import_group.attribute("Label");
        if (label_attr && std::string(label_attr.value()) == "PropertySheets")
        {
            for (pugi::xml_node import_node : import_group.children("Import"))
            {
                if (import_node.attribute("Project").as_string() == props_file)
                {
                    import_group.remove_child(import_node);
                    return 0;
                }
            }
        }
    }

    if (!property_sheets_found)
    {
        pugi::xml_node import_group = project.append_child("ImportGroup");
        import_group.append_attribute("Label") = "PropertySheets";
    }

    return 1;
}

std::string props::get_attr_by_name(const std::string &name, const props_attr_preset &preset)
{
    if (!check())
    {
        return "";
    }

    std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);
    std::string condition = conf.first;
    std::string node_name = conf.second;

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        if (group.attribute("Condition").value() == "'$(Configuration)|$(Platform)'=='" + condition + "'")
        {
            pugi::xml_node node = group.child(node_name);
            pugi::xml_node subnode = node.child(name);

            if (subnode)
            {
                return subnode.text().get();
            }

            break;
        }
    }

    return "";
}

std::string props::get_attr_by_name(const std::string &name, const std::string& condition, bool isClCompile)
{
    if (!check())
    {
        return "";
    }

    std::string node_name = isClCompile?"ClCompile":"Link";

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        if (group.attribute("Condition").value() == "'$(Configuration)|$(Platform)'=='" + condition + "'")
        {
            pugi::xml_node node = group.child(node_name);
            pugi::xml_node subnode = node.child(name);

            if (subnode)
            {
                return subnode.text().get();
            }

            break;
        }
    }

    return "";
}

bool props::set_attr_by_name(const std::string &name, const std::string &value, const props_attr_preset &preset)
{
    if (!check())
    {
        return false;
    }

    std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);
    std::string condition = conf.first;
    std::string node_name = conf.second;

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        if (group.attribute("Condition").value() == "'$(Configuration)|$(Platform)'=='" + condition + "'")
        {
            pugi::xml_node node = group.child(node_name);
            pugi::xml_node subnode = node.child(name);

            if (!subnode)
            {
                subnode = node.append_child(name);
            }

            subnode.text().set(value.c_str());
            return true;
        }
    }

    return false;
}

bool props::set_attr_by_name(const std::string &name, const std::string &value, const std::string& condition, bool isClCompile)
{
    if (!check())
    {
        return false;
    }

    std::string node_name = isClCompile?"ClCompile":"Link";

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        if (group.attribute("Condition").value() == "'$(Configuration)|$(Platform)'=='" + condition + "'")
        {
            pugi::xml_node node = group.child(node_name);
            pugi::xml_node subnode = node.child(name);

            if (!subnode)
            {
                subnode = node.append_child(name);
            }

            subnode.text().set(value.c_str());
            return true;
        }
    }

    return false;
}

bool props::remove_attr_by_name(const std::string &name, const props_attr_preset &preset)
{
    if (!check())
    {
        return false;
    }

    std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);
    std::string condition = conf.first;
    std::string node_name = conf.second;

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        if (group.attribute("Condition").value() == "'$(Configuration)|$(Platform)'=='" + condition + "'")
        {
            pugi::xml_node node = group.child(node_name);
            pugi::xml_node subnode = node.child(name);

            if (subnode)
            {
                node.remove_child(subnode);
            }

            return true;
        }
    }

    return false;
}

bool props::set_attr(const std::string &name, std::string &value, const props_attr_preset &preset)
{
    std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);

    pugi::xpath_node node = m_doc.select_node(
        ("/Project/ItemDefinitionGroup[@Condition=\"'$(Configuration)|$(Platform)'=='" + conf.first + "'\"]/" + conf.second + "/" + name).c_str());

    if (node.node())
    {
        node.node().text().set(value.c_str());
        return true;
    }

    return false;
}

bool props::remove_attr(const std::string &name, const props_attr_preset &preset)
{
    std::pair<std::string, std::string> conf = get_attr_conf_by_preset(preset);

    pugi::xpath_node node = m_doc.select_node(
        ("/Project/ItemDefinitionGroup[@Condition=\"'$(Configuration)|$(Platform)'=='" + conf.first + "'\"]/" + conf.second + "/" + name).c_str());

    if (node.node())
    {
        node.node().parent().remove_child(node.node());
        return true;
    }

    return false;
}

std::vector<std::string> props::get_conditions()
{
    if (!check())
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> conditions;

    pugi::xml_node project = m_doc.child("Project");

    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        std::string str=group.attribute("Condition").value();
        size_t end=str.rfind('\'');
        size_t begin=str.rfind('\'',end-1);
        
        if(end==std::string::npos||begin==std::string::npos)
        {
            continue;
        }

        std::string condition(str.begin()+begin+1,str.begin()+end);
        conditions.push_back(condition);
    }

    return conditions;
}

void props::create(const attribute_table_config &debug_conf, const attribute_table_config &release_conf)
{
    create_root();
    create_item_group(debug_conf);
    create_item_group(release_conf);
}

void props::create_root()
{
    // 添加 XML 声明
    pugi::xml_node declaration = m_doc.append_child(pugi::node_declaration);
    declaration.append_attribute("version") = "1.0";
    declaration.append_attribute("encoding") = "utf-8";

    pugi::xml_node project = m_doc.append_child("Project");
    project.append_attribute("DefaultTargets") = "Build";
    project.append_attribute("xmlns") = "http://schemas.microsoft.com/developer/msbuild/2003";

    // 创建 PropertyGroup
    pugi::xml_node propertyGroup = project.append_child("PropertyGroup");
    propertyGroup.append_attribute("Label") = "UserMacros";
}

bool props::create_item_group(const attribute_table_config &conf)
{
    const configuration_type &configuration = conf.build_type;
    const platform_type &platform = conf.platform;

    pugi::xml_node project = m_doc.child("Project");
    pugi::xml_node group = project.append_child("ItemDefinitionGroup");

    std::string condition = get_condition(conf.build_type, conf.platform);
    if (condition.empty())
    {
        return false;
    }

    group.append_attribute("Condition") = "'$(Configuration)|$(Platform)'=='" + condition + "'";

    pugi::xml_node clcompile = group.append_child("ClCompile");
    clcompile.append_child("WarningLevel").text().set("Level3");
    clcompile.append_child("SDLCheck").text().set("true");

    clcompile.append_child("AdditionalOptions").text().set("%(AdditionalOptions) " + combine_attribute_value(conf.addl_options, " "));
    clcompile.append_child("PreprocessorDefinitions").text().set("%(PreprocessorDefinitions)" + combine_attribute_value(conf.preprocessor_defs));
    // clcompile.append_child("ConformanceMode").text().set("true");
    clcompile.append_child("AdditionalIncludeDirectories").text().set("%(AdditionalIncludeDirectories)" + combine_attribute_value(conf.addl_inc_dirs));

    pugi::xml_node link = group.append_child("Link");
    link.append_child("SubSystem").text().set("Console");
    // link.append_child("GenerateDebugInformation").text().set("true");
    link.append_child("AdditionalLibraryDirectories").text().set(combine_attribute_value(conf.addl_lib_dirs));
    link.append_child("AdditionalDependencies").text().set("$(CoreLibraryDependencies);%(AdditionalDependencies)" + combine_attribute_value(conf.addl_deps));

    return true;
}

void props::print_content()
{
    aDebug(AZH_INFO) << "==============================================================";
    aDebug(AZH_INFO) << "Print xml content :";
    pugi::xml_node project = m_doc.child("Project");

    // 解析所有 ItemDefinitionGroup
    for (pugi::xml_node group : project.children("ItemDefinitionGroup"))
    {
        aDebug(AZH_INFO) << "--------------------------------------------------------------";
        std::string condition = group.attribute("Condition").value();
        aDebug(AZH_INFO) << "Codition : " << condition;

        // 解析 ClCompile 设置
        pugi::xml_node clCompile = group.child("ClCompile");
        if (clCompile)
        {
            aDebug(AZH_INFO) << "ClCompile :";
            aDebug(AZH_INFO) << "  - WarningLevel: " << clCompile.child("WarningLevel").text().get();
            aDebug(AZH_INFO) << "  - SDLCheck: " << clCompile.child("SDLCheck").text().get();
            aDebug(AZH_INFO) << "  - PreprocessorDefinitions: " << clCompile.child("PreprocessorDefinitions").text().get();
            aDebug(AZH_INFO) << "  - AdditionalIncludeDirectories: " << clCompile.child("AdditionalIncludeDirectories").text().get();
        }

        // 解析 Link 设置
        pugi::xml_node link = group.child("Link");
        if (link)
        {
            aDebug(AZH_INFO) << "Link :";
            aDebug(AZH_INFO) << "  - SubSystem: " << link.child("SubSystem").text().get();
            aDebug(AZH_INFO) << "  - AdditionalLibraryDirectories: " << link.child("AdditionalLibraryDirectories").text().get();
            aDebug(AZH_INFO) << "  - AdditionalDependencies: " << link.child("AdditionalDependencies").text().get();
        }
    }

    aDebug(AZH_INFO) << "--------------------------------------------------------------";
}