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

    props get_props() { return m_data; }

public slots:
    void load_props(const props& p);
    void save(const QString& file_path="");

signals:
    void rename(attribute_table_widget*,const QString&);
    void save_props();
    void close_props();
    void exit();

private:
    Ui::attribute_table_widget *ui;
    attribute_table_status m_state;
    props m_data;
    attribute_table_conf m_curr_conf;
    attribute_table_view m_curr_view;
    QMap<QString,QString> m_attr_cache;

    void init();
    void init_action();
    void init_conf();

    void clean();
    void refresh();

    void switch_view();
};

#endif // ATTRIBUTE_TABLE_H
