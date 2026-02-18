#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QAction>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>

#include "attribute_table_widget.h"
#include "azh/props.h"
#include "azh/utils/logger.hpp"
#include "azh/version.hpp"
#include "file_selector_dialog.h"
#include "library_auto_scan_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_library_scanner(new library_auto_scan_widget)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::props_editor_rename(attribute_table_widget *w,
                                     const QString &name)
{
    int index = ui->props_editors->indexOf(w);
    if (index != -1)
    {
        ui->props_editors->setCurrentIndex(index);
        ui->props_editors->setTabText(index, name);
    }
    else
    {
        aDebug(AZH_INFO) << "The widget not found in tab widget.";
    }
}

/* new props_editor, set title to "空白属性表" */
void MainWindow::new_attribute_table_wdiget_for_win64()
{
    new_attribute_table_wdiget_by_props(props());
}

void MainWindow::new_attribute_table_wdiget_for_win32()
{
    props p(1);
    new_attribute_table_wdiget_by_props(p);
}

/* new props_editor, and set props's filename as title, if filename is empty,
 * set title to "空白属性表" */
void MainWindow::new_attribute_table_wdiget_by_props(const props &p)
{
    attribute_table_widget *w = new attribute_table_widget;
    connect(w, &attribute_table_widget::rename, this,
            &MainWindow::props_editor_rename);
    connect(w, &attribute_table_widget::exit, this,
            &MainWindow::remove_attribute_table_widget);

    attribute_table_widget *currWidget = w;
    if (ui->props_editors->count() == 1)
    {
        currWidget = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
    }

    if (currWidget && !currWidget->is_load() && p.is_load())
    {
        currWidget->load_props(p);
    }
    else
    {
        ui->props_editors->addTab(w, "");
        w->load_props(p);
    }
}

void MainWindow::new_attribute_table_wdiget_by_scanner()
{
    std::string incPaths =
        m_library_scanner->get_inc_paths().join(";").toStdString();
    std::string libPaths =
        m_library_scanner->get_lib_paths().join(";").toStdString();

    std::string libNames =
        m_library_scanner->get_lib_names().join(";").toStdString();

    std::string incRelPaths =
        m_library_scanner->get_inc_paths(false).join(";").toStdString();
    std::string libRelPaths =
        m_library_scanner->get_lib_paths(false).join(";").toStdString();

    std::string libRelNames =
        m_library_scanner->get_lib_names(false).join(";").toStdString();

    props p;

    if (m_library_scanner->get_platform() == "x64")
    {
        p.set_attr("AdditionalIncludeDirectories", incPaths,
                   props_attr_preset::DEBUG_WIN64_CLCOMPILE);
        p.set_attr("AdditionalIncludeDirectories", incRelPaths,
                   props_attr_preset::RELEASE_WIN64_CLCOMPILE);

        p.set_attr("AdditionalLibraryDirectories", libPaths,
                   props_attr_preset::DEBUG_WIN64_LINK);
        p.set_attr("AdditionalLibraryDirectories", libRelPaths,
                   props_attr_preset::RELEASE_WIN64_LINK);

        p.set_attr("AdditionalDependencies", libNames,
                   props_attr_preset::DEBUG_WIN64_LINK);
        p.set_attr("AdditionalDependencies", libRelNames,
                   props_attr_preset::RELEASE_WIN64_LINK);
    }
    else if (m_library_scanner->get_platform() == "x86")
    {
        p.set_attr("AdditionalIncludeDirectories", incPaths,
                   props_attr_preset::DEBUG_WIN32_CLCOMPILE);
        p.set_attr("AdditionalIncludeDirectories", incRelPaths,
                   props_attr_preset::RELEASE_WIN32_CLCOMPILE);

        p.set_attr("AdditionalLibraryDirectories", libPaths,
                   props_attr_preset::DEBUG_WIN32_LINK);
        p.set_attr("AdditionalLibraryDirectories", libRelPaths,
                   props_attr_preset::RELEASE_WIN32_LINK);

        p.set_attr("AdditionalDependencies", libNames,
                   props_attr_preset::DEBUG_WIN32_LINK);
        p.set_attr("AdditionalDependencies", libRelNames,
                   props_attr_preset::RELEASE_WIN32_LINK);
    }
    else
    {
        return;
    }

    new_attribute_table_wdiget_by_props(p);

    attribute_table_widget *w = static_cast<attribute_table_widget *>(
        ui->props_editors->currentWidget());

    w->set_edit_state(true);

    m_library_scanner->clear();
}

void MainWindow::remove_attribute_table_widget()
{
    int n = ui->props_editors->count();
    attribute_table_widget *w = static_cast<attribute_table_widget *>(
        ui->props_editors->currentWidget());
    if (n > 1)
    {
        int index = ui->props_editors->indexOf(w);
        if (index != -1)
        {
            ui->props_editors->removeTab(index);
        }
        else
        {
            aDebug(AZH_INFO) << "The widget not found in tab widget.";
        }
    }
    else
    {
        w->load_props(props());
        QMessageBox::about(this, "提示",
                           "已经是最后一个属性表，已为您执行清空操作");
    }
}

void MainWindow::open_global_win64_attribute_table()
{ /* 全局属性表所在文件夹 */
    QString globalPropsRoot =
        QDir::homePath() + "/AppData/Local/Microsoft/MSBuild/v4.0";
    props p(qstring2std(globalPropsRoot) + "/" +
                get_user_props_name_by_platform(platform_type::Win64),
            0);
    if (!p.is_load())
    {
        QMessageBox::StandardButton btn = QMessageBox::warning(
            this, "导入全局属性表", "全局属性表路径不存在，是否创建",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (btn == QMessageBox::No)
        {
            return;
        }

        QDir dir;
        dir.mkpath(globalPropsRoot);
        p.save();
        QMessageBox::about(this, "导入全局属性表", "已为您创建 x64 全局属性表");
    }

    new_attribute_table_wdiget_by_props(p);
}

void MainWindow::open_global_win32_attribute_table()
{ /* 全局属性表所在文件夹 */
    QString globalPropsRoot =
        QDir::homePath() + "/AppData/Local/Microsoft/MSBuild/v4.0";

    props p(qstring2std(globalPropsRoot) + "/" +
                get_user_props_name_by_platform(platform_type::Win32),
            1);
    if (!p.is_load())
    {
        QMessageBox::StandardButton btn = QMessageBox::warning(
            this, "导入全局属性表", "全局属性表路径不存在，是否创建",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (btn == QMessageBox::No)
        {
            return;
        }

        QDir dir;
        dir.mkpath(globalPropsRoot);
        p.save();
        QMessageBox::about(this, "导入全局属性表",
                           "已为您创建 Win32 全局属性表");
    }

    new_attribute_table_wdiget_by_props(p);
}

void MainWindow::open_attribute_table_by_props_or_vcxproj()
{
    const QString &filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择 props/vcxproj 文件"), "",
        QStringLiteral("props/project file(*.vcxproj *.props)"));

    if (!filePath.isEmpty())
    {
        props p;

        if (filePath.endsWith(".props"))
        {
            p.load(qstring2std(filePath));
        }
        else if (filePath.endsWith(".vcxproj"))
        {
            p = props::from_project_file(qstring2std(filePath));
        }
        else
        {
            QMessageBox::warning(this, "打开属性表或载入项目属性表",
                                 "未知的文件格式！");
            return;
        }

        if (!p.check())
        {
            QMessageBox::warning(
                this, "打开属性表或载入项目属性表",
                "选择的项目/属性表文件格式有误，请检查文件过后重试！");
            return;
        }

        new_attribute_table_wdiget_by_props(p);

        QMessageBox::about(this, "打开属性表或载入项目属性表", "导入成功！");
    }
}

void MainWindow::open_multi_function_selector()
{
    file_selector_dialog dlg;
    dlg.setWindowIcon(QIcon(":/res/atrribute_table.png"));
    dlg.resize(600, 400);
    dlg.exec();
}

void MainWindow::open_library_auto_scanner() { m_library_scanner->show(); }

void MainWindow::save_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());

        w->save();
    }
}

void MainWindow::save_as_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());

        w->save();
    }
}

void MainWindow::add_additional_dirs_or_deps_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());

        w->add_path_btn_clicked();
    }
}

void MainWindow::open_props_dir_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
        props p = w->get_props();

        if (p.is_load())
        {
            QString props_path = std2qstring(
                std::filesystem::path(p.get_path()).parent_path().string());
            QDesktopServices::openUrl(QUrl::fromLocalFile(props_path));
        }
        else
        {
            QMessageBox::warning(this, "提示",
                                 "当前属性表为空或者属性表路径不存在");
        }
    }
}

void MainWindow::list_sub_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
        props p = w->get_props();

        if (!p.is_load())
        {
            QMessageBox::warning(
                this, "列出子属性表",
                "当前属性表为空或者属性表路径不存在,无法查看子属性表");
            return;
        }

        const std::vector<std::string> &sheets = p.get_property_sheets();
        QStringList subProps;

        for (const auto &s : sheets)
        {
            subProps.push_back(std2qstring(s));
        }

        if (subProps.empty())
        {
            QMessageBox::warning(this, "列出子属性表", "当前属性表无子属性表");
            return;
        }

        QMessageBox::about(this, "列出子属性表",
                           "当前属性表的子属性表有: \n" + subProps.join("\n"));
    }
}

void MainWindow::add_sub_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
        props p = w->get_props();

        if (!p.is_load())
        {
            QMessageBox::warning(
                this, "添加子属性表",
                "当前属性表为空或者属性表路径不存在,无法添加子属性表");
            return;
        }

        const QString &filePath =
            QFileDialog::getOpenFileName(this, QStringLiteral("选择属性表"), "",
                                         QStringLiteral("props file(*.props)"));

        if (filePath.isEmpty())
        {
            QMessageBox::warning(this, "添加子属性表",
                                 "用户取消了 '添加子属性表'");
            return;
        }

        p.add_property_sheet(qstring2std(filePath));
        w->load_props(p);
        w->set_edit_state(true);
        QMessageBox::about(this, "添加子属性表",
                           "添加子属性表成功,默认不保存到文件，请自行保存");
    }
}

void MainWindow::remove_sub_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
        props p = w->get_props();

        if (!p.is_load())
        {
            QMessageBox::warning(
                this, "删除子属性表",
                "当前属性表为空或者属性表路径不存在,无法删除子属性表");
            return;
        }

        const std::vector<std::string> &sheets = p.get_property_sheets();
        QStringList subProps;

        for (const auto &s : sheets)
        {
            subProps.push_back(std2qstring(s));
        }

        if (subProps.empty())
        {
            QMessageBox::warning(this, "删除子属性表",
                                 "当前属性表无子属性表,无需删除子属性表");
            return;
        }

        bool ok;
        QString sub_props_file = QInputDialog::getItem(
            this, "删除子属性表", "请选择要删除的子属性表:", subProps, 0, false,
            &ok);

        if (!ok || sub_props_file.isEmpty())
        {
            QMessageBox::warning(this, "删除子属性表",
                                 "用户取消了 '删除子属性表'");
            return;
        }

        /* remove sub_props_file from current props's property_sheets */
        p.remove_property_sheet(qstring2std(sub_props_file));
        w->load_props(p);
        w->set_edit_state(true);
        QMessageBox::about(this, "删除子属性表",
                           "删除子属性表成功,默认不保存到文件，请自行保存");
    }
}

void MainWindow::cover_props_in_activate_attribute_table()
{
    int n = ui->props_editors->count();
    if (n >= 1)
    {
        attribute_table_widget *w = static_cast<attribute_table_widget *>(
            ui->props_editors->currentWidget());
        props p = w->get_props();

        if (!p.is_load())
        {
            QMessageBox::warning(
                this, "覆盖属性表",
                "当前属性表为空或者属性表路径不存在,无需覆盖该属性表");
            return;
        }

        const QString &filePath = QFileDialog::getOpenFileName(
            this, QStringLiteral("选择属性表用于覆盖当前属性表"), "",
            QStringLiteral("props file(*.props)"));

        if (!filePath.isEmpty())
        {
            props temp_p(qstring2std(filePath));

            if (!temp_p.is_load() || !temp_p.check())
            {
                QMessageBox::warning(this, "覆盖属性表",
                                     "选择的属性表格式异常，终止操作");
                return;
            }

            QString save_path = std2qstring(p.get_path());

            w->load_props(temp_p);
            w->save_as(save_path);
            QMessageBox::about(this, "覆盖属性表", "覆盖成功");
        }
    }
}

void MainWindow::init()
{
    /* AD */
    setWindowTitle(windowTitle()+"，QQ群 : 1078012714（欢迎学习交流、问题反馈、意见反馈）");

    /* open/create win64 global props */
    connect(ui->win64_props_action, &QAction::triggered, this,
            &MainWindow::open_global_win64_attribute_table);

    /* open/create win32 global props */
    connect(ui->win32_props_action, &QAction::triggered, this,
            &MainWindow::open_global_win32_attribute_table);

    /* open props/vcxproj */
    connect(ui->project_props_action, &QAction::triggered, this,
            &MainWindow::open_attribute_table_by_props_or_vcxproj);

    /* new win64 props editor */
    connect(ui->new_win64_props_action, &QAction::triggered, this,
            &MainWindow::new_attribute_table_wdiget_for_win64);

    /* new win32 props editor */
    connect(ui->new_win32_props_action, &QAction::triggered, this,
            &MainWindow::new_attribute_table_wdiget_for_win32);

    /* save props by current props editor */
    connect(ui->save_props_action, &QAction::triggered, this,
            &MainWindow::save_props_in_activate_attribute_table);

    /* save as props by current props editor */
    connect(ui->save_as_props_action, &QAction::triggered, this,
            &MainWindow::save_as_props_in_activate_attribute_table);

    /* add additional dirs or deps by current props editor */
    connect(ui->add_additional_dirs_or_deps_props_action, &QAction::triggered,
            this,
            &MainWindow::
                add_additional_dirs_or_deps_props_in_activate_attribute_table);

    /* close current props editor */
    connect(ui->close_props_action, &QAction::triggered, this,
            &MainWindow::remove_attribute_table_widget);

    /* open current props's directory */
    connect(ui->open_props_path_action, &QAction::triggered, this,
            &MainWindow::open_props_dir_in_activate_attribute_table);

    /* list sub props */
    connect(ui->list_sub_props_action, &QAction::triggered, this,
            &MainWindow::list_sub_props_in_activate_attribute_table);

    /* add sub props */
    connect(ui->add_sub_props_action, &QAction::triggered, this,
            &MainWindow::add_sub_props_in_activate_attribute_table);

    /* remove sub props */
    connect(ui->remove_sub_props_action, &QAction::triggered, this,
            &MainWindow::remove_sub_props_in_activate_attribute_table);

    /* use other props cover current props */
    connect(ui->cover_props_action, &QAction::triggered, this,
            &MainWindow::cover_props_in_activate_attribute_table);

    connect(ui->multi_function_selector_action, &QAction::triggered, this,
            &MainWindow::open_multi_function_selector);

    connect(ui->scan_lib_action, &QAction::triggered, this,
            &MainWindow::open_library_auto_scanner);

    /* links */
    connect(ui->links_action, &QAction::triggered, this,
            [=]() { display_detail_from_qrc("参考链接", "link"); });

    /* custom library cmake template */
    connect(ui->custom_library_cmake_template_action, &QAction::triggered, this,
            [=]() {
                display_detail_from_qrc("常规库 cmake 模板", "cmake_template");
            });

    /* qt cmake template */
    connect(ui->qt_cmake_template_action, &QAction::triggered, this, [=]() {
        display_detail_from_qrc("Qt cmake 模板", "cmake_qt_template");
    });

    /* help */
    connect(ui->help_action, &QAction::triggered, this,
            [=]() { display_detail_from_qrc("使用说明", "help"); });

    /* about current project */
    connect(ui->about_action, &QAction::triggered, this, [=]() {
        display_detail_from_qrc("关于本项目", "about",
                                QString::fromStdString(AZH_VERSION) + "\n");
    });

    /* about Qt */
    connect(ui->about_qt_action, &QAction::triggered, this,
            [=]() { QMessageBox::aboutQt(this); });

    new_attribute_table_wdiget_for_win64();

    m_library_scanner->setWindowIcon(QIcon(":/res/atrribute_table.png"));
    connect(m_library_scanner, &library_auto_scan_widget::scanned, this,
            &MainWindow::new_attribute_table_wdiget_by_scanner);
}

QString MainWindow::read_text_from_qrc(const QString &name)
{
    QFile file(":/res/text/" + name + ".txt");
    file.open(QFileDevice::ReadOnly);

    return file.readAll();
}

void MainWindow::display_detail_from_qrc(const QString &title,
                                         const QString &name,
                                         const QString &supplement)
{
    QMessageBox box(this);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);
    box.setWindowTitle(title);

    QString str = read_text_from_qrc(name) + supplement;
    box.setDetailedText(str);
    box.setText(str);

    box.exec();
}
