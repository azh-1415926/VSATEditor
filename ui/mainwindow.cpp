#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QAction>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QInputDialog>

#include "azh/version.hpp"
#include "azh/props.h"
#include "attribute_table_widget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::props_editor_rename(attribute_table_widget* w,const QString& name)
{
    int index = ui->props_editors->indexOf(w);
    if (index != -1)
    {
        ui->props_editors->setCurrentIndex(index);
        ui->props_editors->setTabText(index,name);
    }
    else
    {
        qDebug() << "The widget not found in tab widget.";
    }
}

/* new props_editor, set title to "空白属性表" */
void MainWindow::new_attribute_table_wdiget()
{
    new_attribute_table_wdiget_by_props(props());
}

/* new props_editor, and set props's filename as title, if filename is empty, set title to "空白属性表" */
void MainWindow::new_attribute_table_wdiget_by_props(const props& p)
{
    attribute_table_widget* w=new attribute_table_widget;
    connect(w,&attribute_table_widget::rename,this,&MainWindow::props_editor_rename);
    connect(w,&attribute_table_widget::exit,this,&MainWindow::remove_attribute_table_widget);

    emit add_props_editor(w,p);
}

void MainWindow::remove_attribute_table_widget()
{
    int n=ui->props_editors->count();
    attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
    if(n>1)
    {
        emit remove_props_editor(w);
    }
    else
    {
        w->load_props(props());
        QMessageBox::about(this,"tips","已经是最后一个属性表，已为您执行清空操作");
    }
}

void MainWindow::init()
{
    /* 全局属性表所在文件夹 */
    QString props_root = QDir::homePath() + "/AppData/Local/Microsoft/MSBuild/v4.0";

    /* open/create win64 global props */
    connect(ui->win64_props_action,&QAction::triggered,this,[=](){
        props p(qstring2std(props_root)+"/"+get_user_props_name_by_platform(platform_type::Win64),0);
        if(!p.is_load())
        {
            QMessageBox::StandardButton btn=QMessageBox::warning(this,"import props","全局属性表路径不存在，是否创建",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);

            if(btn==QMessageBox::No)
            {
                return;
            }

            QDir dir;
            dir.mkpath(props_root);
            p.save();
            QMessageBox::about(this,"import props","已为您创建 x64 全局属性表");
        }
        // p.print_content();
        new_attribute_table_wdiget_by_props(p);
    });

    /* open/create win32 global props */
    connect(ui->win32_props_action,&QAction::triggered,this,[=](){
        props p(qstring2std(props_root)+"/"+get_user_props_name_by_platform(platform_type::Win32),1);
        if(!p.is_load())
        {
            QMessageBox::StandardButton btn=QMessageBox::warning(this,"import props","全局属性表路径不存在，是否创建",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);

            if(btn==QMessageBox::No)
            {
                return;
            }

            QDir dir;
            dir.mkpath(props_root);
            p.save();
            QMessageBox::about(this,"import props","已为您创建 Win32 全局属性表");
        }
        // p.print_content();
        new_attribute_table_wdiget_by_props(p);
    });

    /* open props/vcxproj */
    connect(ui->project_props_action,&QAction::triggered,this,[=](){

        const QString& filepath=QFileDialog::getOpenFileName(this, QStringLiteral("select props/project file"), "",QStringLiteral("props/project file(*.vcxproj *.props)"));
        
        if(!filepath.isEmpty())
        {
            props p;

            if(filepath.endsWith(".props"))
            {
                p.load(qstring2std(filepath));
            }
            else if(filepath.endsWith(".vcxproj"))
            {
                p=props::from_project_file(qstring2std(filepath));
            }
            else
            {
                QMessageBox::warning(this,"open props/project","未知的文件格式！");
                return;
            }

            if(!p.check())
            {
                QMessageBox::warning(this,"open props/project","选择的项目/属性表文件格式有误，请检查文件过后重试！");
                return;
            }

            new_attribute_table_wdiget_by_props(p);

            QMessageBox::about(this,"open props/project","导入成功！");
        }
    });

    /* new props editor */
    connect(ui->new_props_action,&QAction::triggered,this,&MainWindow::new_attribute_table_wdiget);

    /* save props by current props editor */
    connect(ui->save_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            w->save();
        }
    });

    /* close current props editor */
    connect(ui->close_props_action,&QAction::triggered,this,&MainWindow::remove_attribute_table_widget);

    /* new props editor */
    connect(this,&MainWindow::add_props_editor,this,[=](attribute_table_widget* w,const props& p){
        attribute_table_widget* curr_w=nullptr;
        if(ui->props_editors->count()==1)
        {
            curr_w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
        }

        if(curr_w&&!curr_w->is_load()&&p.is_load())
        {
            curr_w->load_props(p);
        }
        else
        {
            ui->props_editors->addTab(w,"");
            w->load_props(p);
        }
    });

    /* close current props editor */
    connect(this,&MainWindow::remove_props_editor,this,[=](attribute_table_widget* w){
        int index = ui->props_editors->indexOf(w);
        if (index != -1)
        {
            ui->props_editors->removeTab(index);
        }
        else
        {
            qDebug() << "The widget not found in tab widget.";
        }
        
    });

    /* open current props's directory */
    connect(ui->open_props_path_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            if(p.is_load())
            {
                QString props_path=std2qstring(std::filesystem::path(p.get_path()).parent_path().string());
                QDesktopServices::openUrl(QUrl::fromLocalFile(props_path));
            }
            else
            {
                QMessageBox::warning(this,"tips","当前属性表为空或者属性表路径不存在");
            }
        }
    });

    /* list sub props */
    connect(ui->list_sub_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            if(!p.is_load())
            {
                QMessageBox::warning(this,"list sub props","当前属性表为空或者属性表路径不存在,无法查看子属性表");
                return;
            }

            const std::vector<std::string>& sheets=p.get_property_sheets();
            QStringList sub_props;

            for(const auto& s : sheets)
            {
                sub_props.push_back(std2qstring(s));
            }

            if(sub_props.empty())
            {
                QMessageBox::warning(this,"list sub props","当前属性表无子属性表");
                return;
            }

            QMessageBox::about(this,"list sub props","当前属性表的子属性表有: \n"+sub_props.join("\n"));
        }
    });

    /* add sub props */
    connect(ui->add_sub_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            if(!p.is_load())
            {
                QMessageBox::warning(this,"add sub props","当前属性表为空或者属性表路径不存在,无法添加子属性表");
                return;
            }

            const QString& filepath=QFileDialog::getOpenFileName(this, QStringLiteral("add sub props to current props"), "",QStringLiteral("props file(*.props)"));
        
            if(filepath.isEmpty())
            {
                QMessageBox::warning(this,"add sub props","用户取消了 '添加子属性表'");
                return;
            }

            p.add_property_sheet(qstring2std(filepath));
            w->load_props(p);
            w->set_edit_state(true);
            QMessageBox::about(this,"add sub props","添加子属性表成功,默认不保存到文件，请自行保存");
        }
    });

    /* remove sub props */
    connect(ui->remove_sub_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            if(!p.is_load())
            {
                QMessageBox::warning(this,"remove sub props","当前属性表为空或者属性表路径不存在,无法删除子属性表");
                return;
            }

            const std::vector<std::string>& sheets=p.get_property_sheets();
            QStringList sub_props;

            for(const auto& s : sheets)
            {
                sub_props.push_back(std2qstring(s));
            }

            if(sub_props.empty())
            {
                QMessageBox::warning(this,"remove sub props","当前属性表无子属性表,无需删除子属性表");
                return;
            }

            // select sub props

            bool ok;
            QString sub_props_file = QInputDialog::getItem(this,"remove sub props","请选择要删除的子属性表:",sub_props,0,false,&ok);

            if (!ok ||sub_props_file.isEmpty())
            {
                QMessageBox::warning(this,"remove sub props","用户取消了 '删除子属性表'");
                return;
            }

            // remove sub_props_file from current props's property_sheets

            p.remove_property_sheet(qstring2std(sub_props_file));
            w->load_props(p);
            w->set_edit_state(true);
            QMessageBox::about(this,"add sub props","删除子属性表成功,默认不保存到文件，请自行保存");
        }
    });

    /* use other props cover current props */
    connect(ui->cover_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();

            if(!p.is_load())
            {
                QMessageBox::warning(this,"cover current props","当前属性表为空或者属性表路径不存在,无需覆盖该属性表");
                return;
            }

            const QString& filepath=QFileDialog::getOpenFileName(this, QStringLiteral("select props file to cover current props"), "",QStringLiteral("props file(*.props)"));
        
            if(!filepath.isEmpty())
            {
                props temp_p(qstring2std(filepath));

                if(!temp_p.is_load()||!temp_p.check())
                {
                    QMessageBox::warning(this,"cover current props","选择的属性表格式异常，终止操作");
                    return;
                }

                QString save_path=std2qstring(p.get_path());

                w->load_props(temp_p);
                w->save_as(save_path);
                QMessageBox::about(this,"cover current props","覆盖成功");
            }
        }
    });

    /* links */
    connect(ui->links_action,&QAction::triggered,this,[=](){
        QMessageBox box(this);
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        QString str="CGAl、Boost、PCL 下载链接参考：\n"
            "1. CGAL gmp : https://github.com/CGAL/cgal/releases/download/v6.1/CGAL-6.1-win64-auxiliary-libraries-gmp-mpfr.zip\n"
            "2. CGAL source : https://github.com/CGAL/cgal/releases/download/v6.1/CGAL-6.1.zip\n"
            "3. Boost : https://archives.boost.io/release/1.89.0/binaries/boost_1_89_0-msvc-14.3-64.exe\n"
            "4. PCL : https://github.com/PointCloudLibrary/pcl/releases/download/pcl-1.15.1/PCL-1.15.1-AllInOne-msvc2022-win64.exe\n"
            "\nGithub 文件加速下载镜像：\n"
            "https://gh-proxy.com/\n"
            "https://github.akams.cn/\n";
        box.setWindowTitle("参考链接");
        box.setDetailedText(str);
        box.setText(str);

        box.exec();
    });

    /* custom library cmake template */
    connect(ui->custom_library_cmake_template_action,&QAction::triggered,this,[=](){
        QMessageBox box(this);
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        QString str=
            "cmake_minimum_required(VERSION 3.5)\n"
            "project(Custom_Library_Demo)\n"
            "\n"
            "set(CMAKE_CXX_STANDARD 17)\n"
            "\n"
            "add_compile_options(\"$<$<C_COMPILER_ID:MSVC>:/utf-8>\")\n"
            "add_compile_options(\"$<$<CXX_COMPILER_ID:MSVC>:/utf-8>\")\n"
            "\n"
            "find_package(Custom_Library REQUIRED)\n"
            "\n"
            "include_directories(${Custom_Library_INCLUDE_DIRS})\n"
            "\n"
            "add_executable(${PROJECT_NAME} main.cpp)\n"
            "target_link_libraries(${PROJECT_NAME} PRIVATE ${Custom_Library_LIBRARIES})";
        box.setWindowTitle("常规库 cmake 模板");
        box.setDetailedText(str);
        box.setText(str);

        box.exec();
    });

    /* qt cmake template */
    connect(ui->qt_cmake_template_action,&QAction::triggered,this,[=](){
        QMessageBox box(this);
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        QString str=
            "cmake_minimum_required(VERSION 3.5)\n"
            "project(Qt_Demo)\n"
            "\n"
            "set(CMAKE_INCLUDE_CURRENT_DIR ON)\n"
            "set(CMAKE_AUTOUIC ON)\n"
            "set(CMAKE_AUTOMOC ON)\n"
            "set(CMAKE_AUTORCC ON)\n"
            "set(CMAKE_CXX_STANDARD 17)\n"
            "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
            "\n"
            "add_compile_options(\"$<$<C_COMPILER_ID:MSVC>:/utf-8>\")\n"
            "add_compile_options(\"$<$<CXX_COMPILER_ID:MSVC>:/utf-8>\")\n"
            "\n"
            "find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)\n"
            "find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)\n"
            "\n"
            "file(GLOB_RECURSE srcs ${CMAKE_CURRENT_LIST_DIR}/*.c* ${CMAKE_CURRENT_LIST_DIR}/*.h*)\n"
            "file(GLOB_RECURSE qrcs ${CMAKE_CURRENT_LIST_DIR}/*.qrc)\n"
            "\n"
            "list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS \".\")\n"
            "\n"
            "set(PROJECT_SOURCES\n"
            "   ${srcs}\n"
            "   ${qrcs}\n"
            ")\n"
            "add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})\n"
            "target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)";

        box.setWindowTitle("Qt cmake 模板");
        box.setDetailedText(str);
        box.setText(str);

        box.exec();
    });

    /* help */
    connect(ui->help_action,&QAction::triggered,this,[=](){
        QMessageBox box(this);
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        box.setWindowTitle("使用说明");
        box.setText("假如要全局配置 CGAL 库：\n"
            "1.下载 GMP and MPFR libraries, for Windows 64bits\n"
            "2.下载安装 Boost 库（1.70-1.90 均可）\n"
            "3.下载 CGAL 6.1 源码，编译安装\n"
            "4.使用本工具，新建一个属性表，在 C/C++ 的附加包含目录中添加 gmp、boost、cgal 的头文件路径\n"
            "5.使用本工具，在 C/C++ 的语言标准中输入 stdcpp17\n"
            "6.使用本工具，在链接器的附加依赖项中添加 gmp、boost 的库文件\n"
            "7.使用本工具，保存属性表到合适的路径下\n"
            "8.使用本工具，打开全局 x64 属性表，在菜单属性表->子属性表->导入，并选择刚刚保存好的属性表\n"
            "9.将 gmp、boost 的 dll 所在目录加到环境变量 Path 中\n"
        );

        box.exec();
    });

    /* about current project */
    connect(ui->about_action,&QAction::triggered,this,[=](){
        QMessageBox box(this);
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        box.setWindowTitle("About");
        box.setDetailedText("https://github.com/azh-1415926/VSATEditor");
        box.setText("本软件用于导入、编辑、导出 VS 属性表,欢迎各位提建议\n"
                    "版本       : "+QString::fromStdString(AZH_VERSION)+"\n"
                    "作者       : azh\n"
                    "项目链接 : https://github.com/azh-1415926/VSATEditor"
        );

        box.exec();
    });

    /* about Qt */
    connect(ui->about_qt_action,&QAction::triggered,this,[=](){
        QMessageBox::aboutQt(this);
    });
    
    new_attribute_table_wdiget();
}
