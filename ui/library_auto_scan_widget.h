#ifndef LIB_AUTO_SCAN_WIDGET_H
#define LIB_AUTO_SCAN_WIDGET_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class library_auto_scan_widget;
}
QT_END_NAMESPACE

class library_auto_scan_widget : public QWidget
{
    Q_OBJECT

  public:
    library_auto_scan_widget(QWidget *parent = nullptr);
    ~library_auto_scan_widget();

    QStringList get_inc_paths();
    QStringList get_lib_paths();
    QStringList get_lib_names();
    QString get_platform();

  public slots:
    void scan_complete();
    void clear();

  private slots:
    void scan_library();

  signals:
    void scanned();

  private:
    Ui::library_auto_scan_widget *ui;

    void init();
    bool is_file_or_dir_exists(const QString &path);
    bool is_valid_by_lib_root_dir(const QString &rootDir);

    void scan_stanard(const QString &rootDir);
    void scan_boost(const QString &rootDir);
    void scan_vtk(const QString &rootDir);
    void scan_pcl(const QString &rootDir);
    void scan_lib_in_vcpkg_installed(const QString &vcpkgRootDir,
                                     const QString &triplet);

    QString get_specific_inc_path(const QString &rootDir,
                                  const QString &libName = "");
    QString get_specific_version(const QString &rootDir,
                                 const QString &libName);
    QString get_specific_lib_path(const QString &rootDir);
    QStringList get_lib_names(const QString &libPath,
                              const QString &filter = "");
};

#endif // LIB_AUTO_SCAN_WIDGET_H
