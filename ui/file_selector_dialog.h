#ifndef FILE_SELECTOR_DIALOG_H
#define FILE_SELECTOR_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class file_selector_dialog;
}
QT_END_NAMESPACE

enum file_selector_dialog_mode
{
    FS_FILE,
    FS_DIRECTORY
};

class file_selector_dialog : public QDialog
{
    Q_OBJECT

  public:
    file_selector_dialog(const file_selector_dialog_mode &mode =
                             file_selector_dialog_mode::FS_DIRECTORY,
                         QWidget *parent = nullptr);
    ~file_selector_dialog();

    QStringList getExistingDirectories(const QString &title,
                                       const QString &path);

  private slots:
    void single_select();
    void multi_select();
    void exit();

  signals:

  private:
    Ui::file_selector_dialog *ui;
    file_selector_dialog_mode m_mode_private;

    void init();
};

#endif // FILE_SELECTOR_DIALOG_H
