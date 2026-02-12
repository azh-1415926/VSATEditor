#include "attribute_table_widget.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

#include "./ui_attribute_table_widget.h"
#include "azh/props.h"

/* list editable attributes by default */
static QList<QPair<QString, QString>> default_attributes = {
    QPair<QString, QString>("C/C++ 预处理宏定义",
                            "ClCompile|PreprocessorDefinitions"),
    QPair<QString, QString>("C/C++ 附加包含目录",
                            "ClCompile|AdditionalIncludeDirectories"),
    QPair<QString, QString>("C/C++ 其他选项", "ClCompile|AdditionalOptions"),
    QPair<QString, QString>("C/C++ 语言标准", "ClCompile|LanguageStandard"),
    QPair<QString, QString>("链接器 附加库目录",
                            "Link|AdditionalLibraryDirectories"),
    QPair<QString, QString>("链接器 附加依赖项", "Link|AdditionalDependencies"),
    QPair<QString, QString>("链接器 其他选项", "Link|AdditionalOptions")};

attribute_table_widget::attribute_table_widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::attribute_table_widget),
      m_curr_view(attribute_table_view::RAW), m_name("空白属性表")
{
    ui->setupUi(this);

    init();
    init_action();
}

attribute_table_widget::~attribute_table_widget() { delete ui; }

bool attribute_table_widget::is_load() { return m_data.is_load(); }

/* 加载属性表到当前编辑窗口 */
void attribute_table_widget::load_props(const props &p)
{
    m_state = attribute_table_status::NO_EDIT;
    /* 存储属性表 */
    m_data = p;

    /* 初始化配置，例如 Debug/Release、x64/Win32 配置 */
    init_conf();

    /* 获取属性表路径，若存在则以文件名作为窗口名，否则为 '空属性表' */
    std::string xml_path = m_data.get_path();
    QString s = std2qstring(xml_path);

    if (s.isEmpty() ||
        !std::filesystem::exists(std::filesystem::path(xml_path).parent_path()))
    {
        m_name = "空白属性表";
        emit rename(this, m_name);
        return;
    }

    m_name = std2qstring(std::filesystem::path(xml_path).filename().string());
    emit rename(this, m_name);
}

/* 保存当前属性表，默认会弹窗，也可静默保存[可选] */
void attribute_table_widget::save(bool silence)
{
    if (m_state == attribute_table_status::NO_SAVE && m_data.is_load())
    {
        save_as((std2qstring(m_data.get_path())));

        if (!silence)
            QMessageBox::about(this, "save props/project", "保存成功");
        return;
    }
    else if (m_state == attribute_table_status::NO_EDIT && is_load())
    {
        QMessageBox::about(this, "save props/project", "未编辑，无需保存");
        return;
    }

    const QString &filepath = QFileDialog::getSaveFileName(
        this, QStringLiteral("save props/project file"), "",
        QStringLiteral("props file(*.props)"));
    if (filepath.isEmpty())
    {
        if (!silence)
            QMessageBox::warning(this, "save props/project",
                                 "用户取消了 '保存'");
        return;
    }

    if (m_state == attribute_table_status::NO_SAVE)
    {
        save_as(filepath);
        QMessageBox::about(this, "save props/project",
                           "保存成功，已保存至" + filepath);
    }
}

/* 另存为 file_path, 是否重命名当前窗口为该文件名，默认为是 */
void attribute_table_widget::save_as(const QString &file_path, bool to_rename)
{
    m_state = attribute_table_status::NO_EDIT;

    /* 从已编辑的属性中，依次修改属性表中对应属性 */
    for (const QString &attr_name : m_attr_cache.keys())
    {
        std::string value =
            m_attr_cache.value(attr_name).toLocal8Bit().toStdString();

        QStringList list = attr_name.split("|");
        if (list.isEmpty())
        {
            return;
        }

        bool isClCompile = list.at(0) == "ClCompile";
        std::string attr = list.at(1).toLocal8Bit().toStdString();

        std::string condition =
            get_condition(m_curr_conf.configuration, m_curr_conf.platform);
        m_data.set_attr_by_name(attr, value, condition, isClCompile);
    }

    /* props's save */
    m_data.save(file_path.toLocal8Bit().toStdString());

    /* 获取属性表完整路径中属性表的文件名，例如
     * test.props，用作当前属性表编辑窗口的名称 */
    if (to_rename)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        m_name =
            file_path.last(file_path.size() - file_path.lastIndexOf("/") - 1);
        emit rename(this, m_name);
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        const QStringList &list = file_path.split("/");
        if (!file_path.isEmpty() && !list.isEmpty())
        {
            const QStringList &list = file_path.split("/");
            m_name = list.at(list.size() - 1);
            emit rename(this, m_name);
        }
#endif
    }

    refresh();
}

void attribute_table_widget::init()
{
    /* 初始化为 unload */
    m_state = attribute_table_status::NO_LOAD;
    emit rename(this, m_name);

    /* oringal不可编辑 */
    ui->original_text->setReadOnly(true);
    ui->original_muti_lines_text->setReadOnly(true);

    /* 将默认可编辑属性加入窗口展示框中 */
    QStringList attr_names;

    for (const auto &attr : default_attributes)
    {
        attr_names.push_back(attr.first);
    }

    ui->attrs->addItems(attr_names);
    /* 当 QListWidget 被点击，则更新当前被点击属性的内容 */
    connect(ui->attrs, &QListWidget::itemClicked, this,
            [=](QListWidgetItem *item) {
                int i = attr_names.indexOf(item->text());
                QString attr = default_attributes[i].second;

                if (i == -1)
                    return;

                ui->alter_text->blockSignals(true);
                ui->alter_text->clear();
                ui->alter_text->blockSignals(false);

                ui->alter_muti_lines_text->blockSignals(true);
                ui->alter_muti_lines_text->clear();
                ui->alter_muti_lines_text->blockSignals(false);

                update_view_content_by_attr(attr);
            });

    /* 当切换 Debug/Release 时，存储当前配置，并清空保存的内容与刷新输入框内容
     */
    connect(ui->configuration_combo, &QComboBox::currentTextChanged, this,
            [=](const QString &s) {
                QStringList configurations_list = {
                    "Debug", "Release", "MinSizeRel", "RelWithDebInfo"};

                int i = configurations_list.indexOf(s);

                switch (i)
                {
                case -1:
                    break;

                case 0:
                    this->m_curr_conf.configuration = configuration_type::Debug;
                    break;

                case 1:
                    this->m_curr_conf.configuration =
                        configuration_type::Release;
                    break;

                case 2:
                    this->m_curr_conf.configuration =
                        configuration_type::MinSizeRel;
                    break;

                case 3:
                    this->m_curr_conf.configuration =
                        configuration_type::RelWithDebInfo;
                    break;

                default:
                    break;
                }

                clean();
                refresh();
            });

    /* 当切换 x64/Win32 时，存储当前配置，并清空保存的内容与刷新输入框内容 */
    connect(ui->platform_combo, &QComboBox::currentTextChanged, this,
            [=](const QString &s) {
                QStringList platforms_list = {"x64", "Win32"};

                int i = platforms_list.indexOf(s);

                switch (i)
                {
                case -1:
                    break;

                case 0:
                    this->m_curr_conf.platform = platform_type::Win64;
                    break;

                case 1:
                    this->m_curr_conf.platform = platform_type::Win32;
                    break;

                default:
                    break;
                }

                clean();
                refresh();
            });

    /* 当单行内容编辑框，内容更改，则将状态切换为 unsave，并更新窗口名称，附带上
     * * 表明未保存 */
    connect(ui->alter_text, &QTextEdit::textChanged, this, [=]() {
        auto items = ui->attrs->selectedItems();
        if (items.empty())
        {
            return;
        }

        m_state = attribute_table_status::NO_SAVE;
        emit rename(this, m_name + " [*]");

        int i = attr_names.indexOf(items.at(0)->text());
        QString attr = default_attributes.at(i).second;

        QString value = ui->alter_text->toPlainText();
        m_attr_cache.insert(attr, value);
        qDebug() << "cache : " << ui->alter_text->toPlainText();

        ui->alter_muti_lines_text->blockSignals(true);
        ui->alter_muti_lines_text->clear();
        ui->alter_muti_lines_text->blockSignals(false);

        update_view_content_by_attr(attr);
    });

    /* 当多行内容编辑框，内容更改，则将状态切换为 unsave，并更新窗口名称，附带上
     * * 表明未保存 */
    connect(ui->alter_muti_lines_text, &QTextEdit::textChanged, this, [=]() {
        auto items = ui->attrs->selectedItems();
        if (items.empty())
        {
            return;
        }

        m_state = attribute_table_status::NO_SAVE;
        emit rename(this, m_name + " [*]");

        int i = attr_names.indexOf(items.at(0)->text());
        QString attr = default_attributes.at(i).second;

        QString value;
        if (attr != "AdditionalOptions")
        {
            value = ui->alter_muti_lines_text->toPlainText().replace("\n", ";");
        }
        else
        {
            value = ui->alter_muti_lines_text->toPlainText().replace("\n", " ");
        }

        m_attr_cache.insert(attr, value);
        qDebug() << "cache : " << value;

        ui->alter_text->blockSignals(true);
        ui->alter_text->clear();
        ui->alter_text->blockSignals(false);

        update_view_content_by_attr(attr);
    });
}

void attribute_table_widget::init_action()
{
    /* save action */
    connect(ui->save_btn, &QPushButton::clicked, this,
            &attribute_table_widget::save);

    /* save as action */
    connect(ui->save_as_btn, &QPushButton::clicked, this, [=]() {
        const QString &filepath = QFileDialog::getSaveFileName(
            this, QStringLiteral("Save Props Or Project file"), "",
            QStringLiteral("props file(*.props)"));
        if (filepath.isEmpty())
        {
            QMessageBox::warning(this, "save as props", "用户取消了 '另存为'");
            return;
        }

        save_as(filepath);
        QMessageBox::about(this, "save as props",
                           "保存成功，已保存至" + filepath);
    });

    /* exit_nosave action */
    connect(ui->exit_nosave_btn, &QPushButton::clicked, this, [=]() {
        if (m_state == attribute_table_status::NO_SAVE)
        {
            QMessageBox::StandardButton btn = QMessageBox::warning(
                this, "close props editor",
                "你确定要关闭当前属性表吗，该属性表并未保存",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (btn == QMessageBox::No)
            {
                return;
            }
        }

        emit exit();
    });

    /* add additional directories or additional libraries */
    connect(ui->add_path_btn, &QPushButton::clicked, this, [=]() {
        auto items = ui->attrs->selectedItems();
        if (items.empty())
        {
            QMessageBox::warning(this, "add directories/dependencies path",
                                 "未选择任何属性，无法添加");
            return;
        }

        /* 当前展示的属性 */
        QStringList attr_names;

        for (const auto &attr : default_attributes)
        {
            attr_names.push_back(attr.first);
        }

        int i = attr_names.indexOf(items.at(0)->text());
        QString attr = default_attributes.at(i).second;

        if (attr == "ClCompile|AdditionalIncludeDirectories" ||
            attr == "Link|AdditionalLibraryDirectories")
        {
            QString dir_path = QFileDialog::getExistingDirectory(
                this, "add directories/dependencies path", QDir::currentPath());
            if (dir_path.isEmpty())
            {
                QMessageBox::warning(this, "add directories/dependencies path",
                                     "未选择附加目录/附加依赖项");
                return;
            }

            if (ui->alter_text->toPlainText().isEmpty())
                ui->alter_text->setText(dir_path);
            else
                ui->alter_text->setText(ui->alter_text->toPlainText() + ";" +
                                        dir_path);
        }
        else if (attr == "Link|AdditionalDependencies")
        {
            QString libs_path = QFileDialog::getOpenFileNames(
                                    this, "add directories/dependencies path",
                                    QDir::currentPath(), "*.lib")
                                    .join(";");
            if (libs_path.isEmpty())
            {
                QMessageBox::warning(this, "add directories/dependencies path",
                                     "未选择附加目录/附加依赖项");
                return;
            }

            if (ui->alter_text->toPlainText().isEmpty())
                ui->alter_text->setText(libs_path);
            else
                ui->alter_text->setText(ui->alter_text->toPlainText() + ";" +
                                        libs_path);
        }
        else
        {
            QMessageBox::warning(this, "add directories/dependencies path",
                                 "该属性无需添加附加目录/附加依赖项");
        }
    });

    /* switch view action */
    connect(ui->view_combo, &QComboBox::currentIndexChanged, this, [=](int i) {
        switch (i)
        {
        case 0:
            m_curr_view = attribute_table_view::RAW;
            break;

        case 1:
            m_curr_view = attribute_table_view::MUTI_LINES;
            break;

        default:
            m_curr_view = attribute_table_view::RAW;
            break;
        }

        switch_view();
    });
}

void attribute_table_widget::init_conf()
{
    /* 更新当前属性表的配置信息 */
    std::vector<std::string> conditions = m_data.get_conditions();

    QStringList configurations_list = {"Debug", "Release", "MinSizeRel",
                                       "RelWithDebInfo"};
    QStringList platforms_list = {"x64", "Win32"};

    QList<QString> configurations;
    QList<QString> platforms;

    for (const std::string &condition : conditions)
    {
        QString str = QString::fromStdString(condition);

        for (const QString &s : configurations_list)
        {
            int c_i = str.indexOf(s);

            if (c_i != -1)
            {
                if (!configurations.contains(s))
                {
                    configurations.append(s);
                }

                break;
            }
        }

        for (const QString &s : platforms_list)
        {
            int p_i = str.indexOf(s);

            if (p_i != -1)
            {
                if (!platforms.contains(s))
                {
                    platforms.append(s);
                }

                break;
            }
        }
    }

    ui->configuration_combo->clear();
    ui->configuration_combo->addItems(configurations);

    ui->platform_combo->clear();
    ui->platform_combo->addItems(platforms);

    ui->view_combo->clear();
    ui->view_combo->addItems(QStringList() << "Single Line" << "Multi Lines");
}

void attribute_table_widget::clean()
{
    m_state = attribute_table_status::NO_EDIT;
    emit rename(this, m_name);
    m_attr_cache.clear();
}

void attribute_table_widget::refresh()
{
    QList<QListWidgetItem *> items = ui->attrs->selectedItems();
    if (!items.isEmpty())
    {
        QString attr_name = items.at(0)->text();

        for (auto &i : default_attributes)
        {
            if (attr_name == i.first)
            {
                QStringList list = i.second.split("|");
                if (list.count() < 2)
                    continue;

                bool isClCompile = list.at(0) == "ClCompile";
                std::string attr = qstring2std(list.at(1));

                std::string condition = get_condition(m_curr_conf.configuration,
                                                      m_curr_conf.platform);
                std::string value =
                    m_data.get_attr_by_name(attr, condition, isClCompile);
                QString text = std2qstring(value);

                ui->original_text->setText(text);

                ui->alter_text->blockSignals(true);
                ui->alter_text->setText(text);
                ui->alter_text->blockSignals(false);

                if (attr != "AdditionalOptions")
                {
                    text.replace(";", "\n");
                }
                else
                {
                    text.replace(" ", "\n");
                }

                ui->original_muti_lines_text->setText(text);

                ui->alter_muti_lines_text->blockSignals(true);
                ui->alter_muti_lines_text->setText(text);
                ui->alter_muti_lines_text->blockSignals(false);
            }
        }
    }
    else
    {
        ui->original_text->clear();
        ui->alter_text->clear();
        ui->original_muti_lines_text->clear();
        ui->alter_muti_lines_text->clear();
    }
}

void attribute_table_widget::switch_view()
{
    switch (m_curr_view)
    {
    case attribute_table_view::RAW:
        ui->attr_views->setCurrentWidget(ui->raw_page);
        break;

    case attribute_table_view::MUTI_LINES:
        ui->attr_views->setCurrentWidget(ui->muti_lines_page);
        break;

    default:
        break;
    };
}

void attribute_table_widget::update_view_content_by_attr(const QString &attr)
{
    QStringList list = attr.split("|");
    if (list.count() < 2)
        return;

    bool isClCompile = list.at(0) == "ClCompile";
    std::string sub_attr = list.at(1).toLocal8Bit().toStdString();

    std::string condition =
        get_condition(m_curr_conf.configuration, m_curr_conf.platform);
    /* value from props file */
    std::string value =
        m_data.get_attr_by_name(sub_attr, condition, isClCompile);

    QString text = std2qstring(value);
    QString cache;

    if (m_attr_cache.contains(attr))
    {
        cache = m_attr_cache.value(attr);
    }

    if (cache.isEmpty())
    {
        cache = text;
    }

    /* single line */
    ui->original_text->setText(text);

    if (m_curr_view != attribute_table_view::RAW ||
        ui->alter_text->toPlainText().isEmpty())
    {
        ui->alter_text->blockSignals(true);
        ui->alter_text->setText(cache);
        ui->alter_text->blockSignals(false);
    }

    /* multi lines */

    if (sub_attr != "AdditionalOptions")
    {
        /* if attr not AdditionalOptions, then split by ';' */
        text = text.replace(";", "\n");
        cache = cache.replace(";", "\n");
    }
    else
    {
        /* if attr is AdditionalOptions, then split by ' ' */
        text = text.replace(" ", "\n");
        cache = cache.replace(" ", "\n");
    }

    ui->original_muti_lines_text->setText(text);

    if (m_curr_view != attribute_table_view::MUTI_LINES ||
        ui->alter_muti_lines_text->toPlainText().isEmpty())
    {
        ui->alter_muti_lines_text->blockSignals(true);
        ui->alter_muti_lines_text->setText(cache);
        ui->alter_muti_lines_text->blockSignals(false);
    }
}
