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
    
    void open_multi_function_selector();
    void open_library_auto_scanner();

  signals:
    void add_props_editor(attribute_table_widget *, const props &);
    void remove_props_editor(attribute_table_widget *);

  private:
    Ui::MainWindow *ui;
    library_auto_scan_widget* m_library_scanner;

    void init();
};

#endif // MAINWINDOW_H
