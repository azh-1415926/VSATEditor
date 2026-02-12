#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

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
    void remove_attribute_table_widget();

  signals:
    void add_props_editor(attribute_table_widget *, const props &);
    void remove_props_editor(attribute_table_widget *);

  private:
    Ui::MainWindow *ui;

    void init();
};

#endif // MAINWINDOW_H
