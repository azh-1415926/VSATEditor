#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QAction>
#include <QDir>
#include <QDebug>
#include <QMessageBox>

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
        remove_props_editor(w);
    }
    else
    {
        w->load_props(props());
        QMessageBox::about(this,"提示","已经是最后一个属性表，已为您执行清空操作");
    }
}

void MainWindow::init()
{
    QString props_root = QDir::homePath() + "/AppData/Local/Microsoft/MSBuild/v4.0";

    connect(ui->win64_props_action,&QAction::triggered,this,[=](){
        props p(props_root.toStdString()+"/"+get_user_props_name_by_platform(platform_type::Win64));
        if(!p.is_load())
        {
            QMessageBox::StandardButton btn=QMessageBox::warning(this,"Import Props","全局属性表路径不存在，是否创建",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);

            if(btn==QMessageBox::No)
            {
                return;
            }

            QDir dir;
            dir.mkpath(props_root);
            p.save();
            QMessageBox::about(this,"Import Props","已为您创建 x64 全局属性表");
        }
        // p.print_content();
        new_attribute_table_wdiget_by_props(p);
    });

    connect(ui->win32_props_action,&QAction::triggered,this,[=](){
        props p(props_root.toStdString()+"/"+get_user_props_name_by_platform(platform_type::Win32));
        if(!p.is_load())
        {
            QMessageBox::StandardButton btn=QMessageBox::warning(this,"Import Props","全局属性表路径不存在，是否创建",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);

            if(btn==QMessageBox::No)
            {
                return;
            }

            QDir dir;
            dir.mkpath(props_root);
            p.save();
            QMessageBox::about(this,"Import Props","已为您创建 Win32 全局属性表");
        }
        // p.print_content();
        new_attribute_table_wdiget_by_props(p);
    });

    connect(ui->project_props_action,&QAction::triggered,this,[=](){

        const QString& filepath=QFileDialog::getOpenFileName(this, QStringLiteral("select props/project file"), "",QStringLiteral("props/project file(*.vcxproj *.props)"));
        
        if(!filepath.isEmpty())
        {
            props p;

            if(filepath.endsWith(".props"))
            {
                p.load(filepath.toStdString());
            }
            else if(filepath.endsWith(".vcxproj"))
            {
                p=props::from_project_file(filepath.toStdString());
            }
            else
            {
                QMessageBox::warning(this,"Open props/project","未知的文件格式！");
                return;
            }

            if(!p.check())
            {
                QMessageBox::warning(this,"Open props/project","选择的项目/属性表文件格式有误，请检查文件过后重试！");
                return;
            }

            new_attribute_table_wdiget_by_props(p);

            QMessageBox::about(this,"导入项目/属性表","导入成功！");
        }
    });

    connect(ui->new_props_action,&QAction::triggered,this,&MainWindow::new_attribute_table_wdiget);

    connect(ui->save_props_action,&QAction::triggered,this,[=](){
        int n=ui->props_editors->count();
        if (n>=1)
        {
            attribute_table_widget* w=static_cast<attribute_table_widget*>(ui->props_editors->currentWidget());
            props p=w->get_props();
            
            const QString& filepath=QFileDialog::getSaveFileName(this, QStringLiteral("save props file"), "",QStringLiteral("props file(*.props)"));
            if(filepath.isEmpty())
            {
                QMessageBox::about(this,"保存属性表","未选择保存路径，已取消");
                return;
            }

            p.save(filepath.toStdString());
            w->load_props(p);
        }
    });

    connect(ui->close_props_action,&QAction::triggered,this,&MainWindow::remove_attribute_table_widget);

    connect(this,&MainWindow::add_props_editor,this,[=](attribute_table_widget* w,const props& p){
        ui->props_editors->addTab(w,"");
        w->load_props(p);
    });

    connect(this,&MainWindow::remove_props_editor,this,[=](attribute_table_widget* w){
        int index = ui->props_editors->indexOf(w);
        if (index != -1)
        {
            // ui->props_editors->setCurrentIndex(index-1);
            ui->props_editors->removeTab(index);
        }
        else
        {
            qDebug() << "The widget not found in tab widget.";
        }
        
    });

    connect(ui->about_action,&QAction::triggered,this,[=](){
        QMessageBox box;
        box.setTextInteractionFlags(Qt::TextSelectableByMouse);
        box.setDetailedText("https://github.com/azh-1415926/VSATEditor");
        box.setWindowTitle("About");
        box.setText("本软件用于导入、编辑、导出 VS 属性表,欢迎各位提建议\n项目链接：https://github.com/azh-1415926/VSATEditor");
        // box.setText();
        // box.show("About","本软件用于导入、编辑、导出 VS 属性表\n仓库链接：https://github.com/azh-1415926/VSATEditor\n欢迎各位提建议");
        box.exec();
    });
    
    new_attribute_table_wdiget();
}
