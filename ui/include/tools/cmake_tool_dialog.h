#ifndef CMAKE_TOOL_DIALOG_H
#define CMAKE_TOOL_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class cmake_tool_dialog;
}
QT_END_NAMESPACE

class cmake_tool_dialog : public QDialog
{
    Q_OBJECT

  public:
    cmake_tool_dialog(QWidget *parent = nullptr);
    ~cmake_tool_dialog();

  private slots:
    void updateSourceDir(const QString &s);
    void updateBuildDir(const QString &s);
    void updateInstallDir(const QString &s);
    void updateCMakeSelector();
    void updateCMakeArgs();
    void updateCMakeCommand();
    void exit();

  signals:

  private:
    Ui::cmake_tool_dialog *ui;
    QString m_source_dir;
    QString m_build_dir;
    QString m_install_dir;
    QList<QPair<QString, QString>> m_args_list;

    void init();
};

#endif // CMAKE_TOOL_DIALOG_H
