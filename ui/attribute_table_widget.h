#ifndef ATTRIBUTE_TABLE_H
#define ATTRIBUTE_TABLE_H

#include <QWidget>
#include <QMap>

#include "azh/props.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class attribute_table_widget;
}
QT_END_NAMESPACE

enum class attribute_table_status
{
    NO_LOAD,
    NO_EDIT,
    NO_SAVE
};

struct attribute_table_conf
{
    configuration_type configuration;
    platform_type platform;
};

enum class attribute_table_view
{
    RAW,
    MUTI_LINES
};

class attribute_table_widget : public QWidget
{
    Q_OBJECT

public:
    attribute_table_widget(QWidget *parent = nullptr);
    ~attribute_table_widget();

    attribute_table_status status() { return m_state; }
    bool is_edit() { return m_state==attribute_table_status::NO_SAVE; }
    void set_edit_state(bool is_edit)
    {
        if(is_edit&&is_load()) 
        {
            m_state=attribute_table_status::NO_SAVE;
            emit rename(this,m_name+" [*]");
        }
        else if(!is_edit&&is_load()) 
        {
            m_state=attribute_table_status::NO_EDIT;
            emit rename(this,m_name);
        }
    }
    bool is_load();

    props get_props() { return m_data; }

public slots:
    void load_props(const props& p);
    void save(bool silence=false);
    void save_as(const QString& file_path,bool to_rename=true);

signals:
    void rename(attribute_table_widget*,const QString&);
    void save_props();
    void close_props();
    void exit();

private:
    Ui::attribute_table_widget *ui;
    /* tabname */
    QString m_name;
    /* props edit state */
    attribute_table_status m_state;
    /* props */
    props m_data;
    /* current configuration(Debug/Release) and platform(x64/Win32) */
    attribute_table_conf m_curr_conf;
    /* view type */
    attribute_table_view m_curr_view;
    /* edited attribute contents cache */
    QMap<QString,QString> m_attr_cache;

    void init();
    void init_action();
    void init_conf();

    void clean();
    void refresh();

    void switch_view();
};

/* std::string convert to QString */
inline QString std2qstring(const std::string& s)
{
    #if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
        return QString::fromLocal8Bit(s);
    #elif (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        return QString::fromLocal8Bit(s.data());
    #endif
}

/* QString convert to std::string */
inline std::string qstring2std(const QString& s)
{
    return s.toLocal8Bit().toStdString();
}

#endif // ATTRIBUTE_TABLE_H
