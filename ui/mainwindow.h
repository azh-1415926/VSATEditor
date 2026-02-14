#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class library_auto_scan_widget;
class attribute_table_widget;
class props;

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void props_editor_rename(attribute_table_widget *, const QString &name);
    void new_attribute_table_wdiget();
    void new_attribute_table_wdiget_by_props(const props &p);
    void new_attribute_table_wdiget_by_scanner();
    void remove_attribute_table_widget();

    /* load global props or common props/vcxproj */
    void open_global_win64_attribute_table();
    void open_global_win32_attribute_table();
    void open_attribute_table_by_props_or_vcxproj();

    void open_multi_function_selector();
    void open_library_auto_scanner();

    void save_props_in_activate_attribute_table();
    void open_props_dir_in_activate_attribute_table();

    /* sub props management */
    void list_sub_props_in_activate_attribute_table();
    void add_sub_props_in_activate_attribute_table();
    void remove_sub_props_in_activate_attribute_table();

    void cover_props_in_activate_attribute_table();

  signals:
    void add_props_editor(attribute_table_widget *, const props &);
    void remove_props_editor(attribute_table_widget *);

  private:
    Ui::MainWindow *ui;
    library_auto_scan_widget *m_library_scanner;

    void init();
    QString read_text_from_qrc(const QString &name);
    void display_detail_from_qrc(const QString &title, const QString &name,
                                 const QString &supplement = "");
};

#endif // MAINWINDOW_H
