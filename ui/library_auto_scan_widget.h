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

    QStringList get_inc_paths(bool toDebug = true);
    QStringList get_lib_paths(bool toDebug = true);
    QStringList get_lib_names(bool toDebug = true);
    QString get_platform();

  public slots:
    void scan_complete();
    void reset();

  private slots:
    void appendIncPath(const QString &path, bool isDebug);
    void appendLibPath(const QString &path, bool isDebug);
    void appendIncPaths(const QStringList &paths, bool isDebug);
    void appendLibPaths(const QStringList &paths, bool isDebug);
    void appendLibNames(const QStringList &names, bool isDebug);
    void scan_library();

  signals:
    void scanned();

  private:
    Ui::library_auto_scan_widget *ui;

    void init();
    bool is_file_or_dir_exists(const QString &path);
    bool is_valid_by_lib_root_dir(const QString &rootDir);

    void scan_stanard(const QString &rootDir);
    void scan_opencv(const QString &rootDir);
    void scan_boost(const QString &rootDir);
    void scan_vtk(const QString &rootDir);
    void scan_pcl(const QString &rootDir);
    void scan_occt(const QString &rootDir);
    void scan_lib_in_vcpkg_installed(const QString &vcpkgRootDir,
                                     const QString &triplet);
    void scan_lib_in_vcpkg_installed(const QString &rootDir);

    QString get_specific_inc_path(const QString &rootDir,
                                  const QString &libName = "");
    QString get_specific_version(const QString &rootDir,
                                 const QString &libName);
    QString get_specific_lib_path(const QString &rootDir);
    QStringList get_lib_names(const QString &libPath,
                              const QString &filter = "");

    /* boost */
    QString get_boost_inc_path(const QString &rootDir);
    QString get_boost_lib_path(const QString &rootDir);
    QStringList get_boost_lib_names(const QString &libPath,
                                    const QString &version, bool isDebug);

    /* vtk */
    QString get_vtk_inc_path(const QString &rootDir);
    QString get_vtk_lib_path(const QString &rootDir);
    QStringList get_vtk_lib_names(const QString &libPath,
                                    const QString &version, bool isDebug);
};

#endif // LIB_AUTO_SCAN_WIDGET_H
