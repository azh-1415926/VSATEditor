#include "attribute_table_widget.h"
#include "./ui_attribute_table_widget.h"

#include "azh/props.h"

#include <filesystem>

#include <QFileDialog>
#include <QMessageBox>

static QList<QPair<QString,QString>> default_attributes={
    QPair<QString,QString>("C/C++ 预处理宏定义","ClCompile|PreprocessorDefinitions"),
    QPair<QString,QString>("C/C++ 附加包含目录","ClCompile|AdditionalIncludeDirectories"),
    QPair<QString,QString>("C/C++ 其他选项","ClCompile|AdditionalOptions"),
    QPair<QString,QString>("链接器 附加库目录","Link|AdditionalLibraryDirectories"),
    QPair<QString,QString>("链接器 附加依赖项","Link|AdditionalDependencies"),
    QPair<QString,QString>("链接器 其他选项","Link|AdditionalOptions")
};

attribute_table_widget::attribute_table_widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::attribute_table_widget), m_curr_view(attribute_table_view::RAW)
{
    ui->setupUi(this);

    init();
    init_action();
}

attribute_table_widget::~attribute_table_widget()
{
    delete ui;
}

void attribute_table_widget::load_props(const props& p)
{
    m_state=attribute_table_status::NO_EDIT;

    m_data=p;

    init_conf();

    std::string xml_path=m_data.get_path();
    QString s=QString::fromStdString(xml_path);

    if(s.isEmpty()||!std::filesystem::exists(std::filesystem::path(xml_path).parent_path()))
    {
        emit rename(this,"空白属性表");
        return;
    }

    emit rename(this,QString::fromStdString(std::filesystem::path(xml_path).filename().string()));
}

void attribute_table_widget::save(const QString& file_path)
{
    m_state=attribute_table_status::NO_EDIT;

    for(const QString& attr_name : m_attr_cache.keys())
    {
        std::string value=m_attr_cache.value(attr_name).toStdString();

        QStringList list=attr_name.split("|");
        if(list.isEmpty())
        {
            return;
        }

        bool isClCompile=list.at(0)=="ClCompile";
        std::string attr=list.at(1).toStdString();

        std::string condition=get_condition(m_curr_conf.configuration,m_curr_conf.platform);
        m_data.set_attr_by_name(attr,value,condition,isClCompile);
    }

    if(file_path.isEmpty())
    {
        m_data.save();
    }
    else
    {
        m_data.save(file_path.toStdString());
        emit rename(this,file_path.last(file_path.size()-file_path.lastIndexOf("/")-1));
    }
}

void attribute_table_widget::init()
{
    m_state=attribute_table_status::NO_LOAD;

    connect(ui->attrs,&QListWidget::itemClicked,this,[=](QListWidgetItem* i){
        // QListWidgetItem* attr=ui->attrs->item(i);
        // // qDebug()<<"Attr : "<<attr->objectName();
        qDebug()<<"Attr : "<<i->text();
    });

    QStringList attr_names;

    for(const auto& attr : default_attributes)
    {
        attr_names.push_back(attr.first);
    }

    ui->attrs->addItems(attr_names);

    connect(ui->attrs,&QListWidget::itemClicked,this,[=](QListWidgetItem* item){
        int i=attr_names.indexOf(item->text());

        if(i==-1)
            return;

        QStringList list=default_attributes[i].second.split("|");
        if(list.count()<2)
            return;

        bool isClCompile=list.at(0)=="ClCompile";
        std::string attr=list.at(1).toStdString();

        std::string condition=get_condition(m_curr_conf.configuration,m_curr_conf.platform);
        std::string value=m_data.get_attr_by_name(attr,condition,isClCompile);
        ui->orignal_text->setText(QString::fromStdString(value));
        ui->orignal_muti_lines_text->setText(QString::fromStdString(value).replace(";","\n"));

        QString cache=m_attr_cache.value(default_attributes[i].second);

        ui->alter_text->blockSignals(true);
        ui->alter_muti_lines_text->blockSignals(true);
        if(!cache.isEmpty())
        {
            ui->alter_text->setText(cache);
            ui->alter_muti_lines_text->setText(cache.replace(";","\n"));
        }
        else
        {
            ui->alter_text->setText(QString::fromStdString(value));
            ui->alter_muti_lines_text->setText(QString::fromStdString(value).replace(";","\n"));
        }
        ui->alter_text->blockSignals(false);
        ui->alter_muti_lines_text->blockSignals(false);
    });

    connect(ui->configuration_combo,&QComboBox::currentTextChanged,this,[=](const QString& s){
        QStringList configurations_list={"Debug","Release","MinSizeRel","RelWithDebInfo"};
    
        int i=configurations_list.indexOf(s);
        
        switch (i)
        {
        case -1:
            break;
        
        case 0:
            this->m_curr_conf.configuration=configuration_type::Debug;
            break;

        case 1:
            this->m_curr_conf.configuration=configuration_type::Release;
            break;

        case 2:
            this->m_curr_conf.configuration=configuration_type::MinSizeRel;
            break;

        case 3:
            this->m_curr_conf.configuration=configuration_type::RelWithDebInfo;
            break;
        
        default:
            break;
        }

        clean();
        refresh();
    });

    connect(ui->platform_combo,&QComboBox::currentTextChanged,this,[=](const QString& s){
        QStringList platforms_list={"x64","Win32"};

        int i=platforms_list.indexOf(s);

        switch (i)
        {
        case -1:
            break;

        case 0:
            this->m_curr_conf.platform=platform_type::Win64;
            break;

        case 1:
            this->m_curr_conf.platform=platform_type::Win32;
            break;
        
        default:
            break;
        }

        clean();
        refresh();
    });

    connect(ui->alter_text,&QTextEdit::textChanged,this,[=](){
        auto items=ui->attrs->selectedItems();
        if(items.empty())
        {
            return;
        }

        m_state=attribute_table_status::NO_SAVE;

        int i=attr_names.indexOf(items.at(0)->text());
        QString attr=default_attributes.at(i).second;

        QString value=ui->alter_text->toPlainText();
        m_attr_cache.insert(attr,value);
        qDebug()<<"cache : "<<ui->alter_text->toPlainText();

        ui->alter_muti_lines_text->blockSignals(true);
        ui->alter_muti_lines_text->setText(value.replace(";","\n"));
        ui->alter_muti_lines_text->blockSignals(false);
    });

    connect(ui->alter_muti_lines_text,&QTextEdit::textChanged,this,[=](){
        auto items=ui->attrs->selectedItems();
        if(items.empty())
        {
            return;
        }

        m_state=attribute_table_status::NO_SAVE;

        int i=attr_names.indexOf(items.at(0)->text());
        QString attr=default_attributes.at(i).second;

        QString value=ui->alter_muti_lines_text->toPlainText().replace("\n",";");
        m_attr_cache.insert(attr,value);
        qDebug()<<"cache : "<<value;
        // refresh();

        ui->alter_text->blockSignals(true);
        ui->alter_text->setText(value.replace("\n",";"));
        ui->alter_text->blockSignals(false);
    });
}

void attribute_table_widget::init_action()
{
    connect(ui->save_btn,&QPushButton::clicked,this,[=](){
        if(m_data.is_load())
        {
            save();
            return;
        }

        if(m_state==attribute_table_status::NO_EDIT)
        {
            QMessageBox::about(this,"Save Props Or Project","未编辑，无需保存");
            return;
        }

        const QString& filepath=QFileDialog::getSaveFileName(this, QStringLiteral("Save Props Or Project File"), "",QStringLiteral("props file(*.props)"));
        if(filepath.isEmpty())
        {
            QMessageBox::warning(this,"Save Props Or Project","用户取消了 '保存'");
            return;
        }

        if(m_state==attribute_table_status::NO_SAVE)
        {
            save(filepath);
        }
    });

    connect(ui->save_as_btn,&QPushButton::clicked,this,[=](){
        const QString& filepath=QFileDialog::getSaveFileName(this, QStringLiteral("Save Props Or Project file"), "",QStringLiteral("props file(*.props)"));
        if(filepath.isEmpty())
        {
            QMessageBox::warning(this,"Save Props Or Project","用户取消了 '另存为'");
            return;
        }

        save(filepath);
    });

    connect(ui->exit_nosave_btn,&QPushButton::clicked,this,[=](){
        if(m_state==attribute_table_status::NO_SAVE)
        {
            QMessageBox::StandardButton btn=QMessageBox::warning(this,"Close Props Editor","你确定要关闭当前属性表吗，该属性表并未保存",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);

            if(btn==QMessageBox::No)
            {
                return;
            }
        }
        
        emit exit();
    });

    connect(ui->view_btn,&QPushButton::clicked,this,[=](){
        m_curr_view=(m_curr_view==attribute_table_view::RAW?attribute_table_view::MUTI_LINES:attribute_table_view::RAW);
        switch_view();
    });
}

void attribute_table_widget::init_conf()
{
    std::vector<std::string> conditions=m_data.get_conditions();

    QStringList configurations_list={"Debug","Release","MinSizeRel","RelWithDebInfo"};
    QStringList platforms_list={"x64","Win32"};

    QList<QString> configurations;
    QList<QString> platforms;

    for(const std::string& condition : conditions)
    {
        QString str=QString::fromStdString(condition);
        
        for(const QString& s : configurations_list)
        {
            int c_i=str.indexOf(s);

            if(c_i!=-1)
            {
                if (!configurations.contains(s))
                {
                    configurations.append(s);
                }

                break;
            }
        }
        
        for(const QString& s : platforms_list)
        {
            int p_i=str.indexOf(s);

            if(p_i!=-1)
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
}

void attribute_table_widget::clean()
{
    m_state=attribute_table_status::NO_EDIT;
    m_attr_cache.clear();
}

void attribute_table_widget::refresh()
{
    QList<QListWidgetItem*> items=ui->attrs->selectedItems();
    if(!items.isEmpty())
    {
        QString attr_name=items.at(0)->text();
        
        for(auto& i : default_attributes)
        {
            if(attr_name==i.first)
            {
                QStringList list=i.second.split("|");
                if(list.count()<2)
                    continue;

                bool isClCompile=list.at(0)=="ClCompile";
                std::string attr=list.at(1).toStdString();

                std::string condition=get_condition(m_curr_conf.configuration,m_curr_conf.platform);
                std::string value=m_data.get_attr_by_name(attr,condition,isClCompile);
                QString text=QString::fromStdString(value);
                
                ui->orignal_text->setText(text);

                ui->alter_text->blockSignals(true);
                ui->alter_text->setText(text);
                ui->alter_text->blockSignals(false);

                text.replace(";","\n");

                ui->orignal_muti_lines_text->setText(text);

                ui->alter_muti_lines_text->blockSignals(true);
                ui->alter_muti_lines_text->setText(text);
                ui->alter_muti_lines_text->blockSignals(false);
            }
        }
    }
    else
    {
        ui->orignal_text->clear();
        ui->alter_text->clear();
        ui->orignal_muti_lines_text->clear();
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