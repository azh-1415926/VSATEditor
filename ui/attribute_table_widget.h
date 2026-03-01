#ifndef ATTRIBUTE_TABLE_H
#define ATTRIBUTE_TABLE_H

#include <QMap>
#include <QWidget>

#include "utils.hpp"
#include "azh/props.h"

class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui
{
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
    bool is_edit() { return m_state == attribute_table_status::NO_SAVE; }
    void set_edit_state(bool toEdit)
    {
        if (toEdit)
        {
            m_state = attribute_table_status::NO_SAVE;
            emit rename(this, m_name + " [*]");
        }
        else
        {
            m_state = attribute_table_status::NO_EDIT;
            emit rename(this, m_name);
        }
    }
    bool is_load();

    props get_props() { return m_data; }

  public slots:
    void load_props(const props &p);
    void save(bool toSilence = false);
    void save_as(const QString &file_path, bool to_rename = true);

    
    void attr_item_clicked(QListWidgetItem *item);
    void configuration_changed(const QString &conf);
    void platform_changed(const QString &platform);
    void single_editor_text_changed();
    void multi_editor_text_changed();

    void exit_without_saving();
    void save_as_btn_clicked();
    void add_path_btn_clicked();

  signals:
    void rename(attribute_table_widget *, const QString &);
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
    QMap<QString, QString> m_attr_cache;
    /* attr items's names */
    QStringList m_attr_names;

    void init();
    void init_action();
    void init_conf();

    void clean();
    void refresh();

    void switch_view(int i);
    void update_view_content_by_attr(const QString &attr);
};

#endif // ATTRIBUTE_TABLE_H
