#include "library_auto_scan_widget.h"
#include "./ui_library_auto_scan_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

std::string qstring2std(const QString &s);
inline QString getenvSafe(const QString &key)
{
    const char *env = std::getenv(qstring2std(key).c_str());
    QString envStr;

    if (env)
    {
        envStr = env;
        envStr.replace("\\", "/");

        if (envStr.at(envStr.size() - 1) == "/")
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            envStr.removeAt(envStr.size() - 1);
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            envStr.remove(envStr.size() - 1, 1);
#endif
        }
    }

    return envStr;
}

library_auto_scan_widget::library_auto_scan_widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::library_auto_scan_widget)
{
    ui->setupUi(this);

    init();
}

library_auto_scan_widget::~library_auto_scan_widget() {}

QStringList library_auto_scan_widget::get_inc_paths(bool toDebug)
{
    QStringList paths;
    if (toDebug)
    {
        paths = ui->incs_edit_dbg->toPlainText().split("\n");
    }
    else
    {
        paths = ui->incs_edit_rel->toPlainText().split("\n");
    }

    for (auto path : paths)
    {
        if (path.isEmpty())
        {
            paths.removeOne(path);
        }
    }

    return paths;
}

QStringList library_auto_scan_widget::get_lib_paths(bool toDebug)
{
    QStringList paths;
    if (toDebug)
    {
        paths = ui->lib_dir_edit_dbg->toPlainText().split("\n");
    }
    else
    {
        paths = ui->lib_dir_edit_rel->toPlainText().split("\n");
    }

    for (auto path : paths)
    {
        if (path.isEmpty())
        {
            paths.removeOne(path);
        }
    }

    return paths;
}

QStringList library_auto_scan_widget::get_lib_names(bool toDebug)
{
    QStringList names;
    if (toDebug)
    {
        names = ui->libs_edit_dbg->toPlainText().split("\n");
    }
    else
    {
        names = ui->libs_edit_rel->toPlainText().split("\n");
    }

    for (auto name : names)
    {
        if (name.isEmpty())
        {
            names.removeOne(name);
        }
    }

    return names;
}

QString library_auto_scan_widget::get_platform()
{
    return ui->platform_combo->currentText();
}

void library_auto_scan_widget::scan_complete()
{
    if (ui->incs_edit_dbg->toPlainText().isEmpty() &&
        ui->lib_dir_edit_dbg->toPlainText().isEmpty() &&
        ui->libs_edit_dbg->toPlainText().isEmpty() &&
        ui->incs_edit_rel->toPlainText().isEmpty() &&
        ui->lib_dir_edit_rel->toPlainText().isEmpty() &&
        ui->libs_edit_rel->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, "扫描库", "未扫描任何库");
        return;
    }
    emit scanned();
    this->close();
}

void library_auto_scan_widget::reset()
{
    ui->lib_combo->setCurrentIndex(0);
    ui->platform_combo->setCurrentIndex(0);

    ui->incs_edit_dbg->clear();
    ui->lib_dir_edit_dbg->clear();
    ui->libs_edit_dbg->clear();

    ui->incs_edit_rel->clear();
    ui->lib_dir_edit_rel->clear();
    ui->libs_edit_rel->clear();
}

void library_auto_scan_widget::appendIncPath(const QString &path, bool isDebug)
{
    appendIncPaths(QStringList() << path, isDebug);
}

void library_auto_scan_widget::appendLibPath(const QString &path, bool isDebug)
{
    appendLibPaths(QStringList() << path, isDebug);
}

void library_auto_scan_widget::appendIncPaths(const QStringList &paths,
                                              bool isDebug)
{
    QString preText;
    if (isDebug && !ui->incs_edit_dbg->toPlainText().isEmpty())
    {
        preText = ui->incs_edit_dbg->toPlainText() + "\n";
    }
    else if (!isDebug && !ui->incs_edit_rel->toPlainText().isEmpty())
    {
        preText = ui->incs_edit_rel->toPlainText() + "\n";
    }

    if (isDebug)
    {
        ui->incs_edit_dbg->setText(preText + paths.join("\n"));
    }
    else
    {
        ui->incs_edit_rel->setText(preText + paths.join("\n"));
    }
}
void library_auto_scan_widget::appendLibPaths(const QStringList &paths,
                                              bool isDebug)
{
    QString preText;
    if (isDebug && !ui->lib_dir_edit_dbg->toPlainText().isEmpty())
    {
        preText = ui->lib_dir_edit_dbg->toPlainText() + "\n";
    }
    else if (!isDebug && !ui->lib_dir_edit_rel->toPlainText().isEmpty())
    {
        preText = ui->lib_dir_edit_rel->toPlainText() + "\n";
    }

    if (isDebug)
    {
        ui->lib_dir_edit_dbg->setText(preText + paths.join("\n"));
    }
    else
    {
        ui->lib_dir_edit_rel->setText(preText + paths.join("\n"));
    }
}

void library_auto_scan_widget::appendLibNames(const QStringList &names,
                                              bool isDebug)
{
    QString preText;
    if (isDebug && !ui->libs_edit_dbg->toPlainText().isEmpty())
    {
        preText = ui->libs_edit_dbg->toPlainText() + "\n";
    }
    else if (!isDebug && !ui->libs_edit_rel->toPlainText().isEmpty())
    {
        preText = ui->libs_edit_rel->toPlainText() + "\n";
    }

    if (isDebug)
    {
        ui->libs_edit_dbg->setText(preText + names.join("\n"));
    }
    else
    {
        ui->libs_edit_rel->setText(preText + names.join("\n"));
    }
}

void library_auto_scan_widget::scan_library()
{
    QString libName = ui->lib_combo->currentText();
    if (libName.isEmpty())
    {
        QMessageBox::warning(this, "扫描库", "未选择需要扫描的库");
        return;
    }

    QString rootDir = QFileDialog::getExistingDirectory(this, "选择库根路径",
                                                        QDir::currentPath());

    if (rootDir.isEmpty())
    {
        QMessageBox::warning(this, "扫描库", "未选择需要扫描的库");
        return;
    }

    if (!is_valid_by_lib_root_dir(rootDir))
    {
        QMessageBox::warning(this, "扫描库",
                             "请选择库的根路径，该路径不符合要求");
        return;
    }

    switch (ui->lib_combo->currentIndex())
    {
    case 0:
        /* Standard */
        scan_stanard(rootDir);
        break;

    case 1:
        /* OpenCV */
        scan_opencv(rootDir);
        break;

    case 2:
        /* Boost */
        scan_boost(rootDir);
        break;

    case 3:
        /* VTK */
        scan_vtk(rootDir);
        break;

    case 4:
        /* PCL */
        scan_pcl(rootDir);
        break;

    case 5:
        /* vcpkg */
        scan_lib_in_vcpkg_installed(rootDir, ui->platform_combo->currentText() +
                                                 "-windows");
        break;

    case 6:
        /* vcpkg */
        scan_lib_in_vcpkg_installed(rootDir, ui->platform_combo->currentText() +
                                                 "-windows-static");
        break;

    default:
        break;
    }
}

void library_auto_scan_widget::init()
{
    connect(ui->scan_btn, &QPushButton::clicked, this,
            &library_auto_scan_widget::scan_library);

    connect(ui->reset_btn, &QPushButton::clicked, this,
            &library_auto_scan_widget::reset);

    connect(ui->ok_btn, &QPushButton::clicked, this,
            &library_auto_scan_widget::scan_complete);

    QStringList libNames = {"常规库",
                            "OpenCV",
                            "Boost",
                            "VTK",
                            "PCL",
                            "vcpkg : windows",
                            "vcpkg : windows-static"};
    ui->lib_combo->addItems(libNames);

    ui->platform_combo->addItems(QStringList() << "x64" << "x86");
}

bool library_auto_scan_widget::is_file_or_dir_exists(const QString &path)
{
    return std::filesystem::exists(qstring2std(path));
}

bool library_auto_scan_widget::is_valid_by_lib_root_dir(const QString &rootDir)
{
    QStringList inc_names = {"include", "Include", "inc", "boost", "installed"};

    for (auto inc : inc_names)
    {
        if (is_file_or_dir_exists(rootDir + "/" + inc))
        {
            return true;
        }
    }

    return false;
}

void library_auto_scan_widget::scan_stanard(const QString &rootDir)
{
    /* first, confirm whether this library was compiled with vcpkg */
    if (is_file_or_dir_exists(rootDir + "/debug/lib"))
    {
        scan_lib_in_vcpkg_installed(rootDir);
        return;
    }

    appendIncPath(get_specific_inc_path(rootDir), true);
    appendIncPath(get_specific_inc_path(rootDir), false);

    QString libPath = get_specific_lib_path(rootDir);
    if (!libPath.isEmpty())
    {
        QStringList libNames = get_lib_names(libPath);
        appendLibPath(libPath, true);
        appendLibPath(libPath, false);
        appendLibNames(libNames, true);
        appendLibNames(libNames, false);
    }
}

void library_auto_scan_widget::scan_opencv(const QString &rootDir)
{
    QString platform = ui->platform_combo->currentText();
    if (!is_file_or_dir_exists(rootDir + "/" + platform))
    {
        QMessageBox::warning(this, "扫描库",
                             "无法找到 opencv 的 " + platform + " 库");
        return;
    }

    QString libPath;
    QDir libDir(rootDir + "/" + platform);
    auto eil = libDir.entryInfoList();

    for (auto e : eil)
    {
        if (e.isDir())
        {
            libPath = e.filePath() + "/lib";
        }
    }

    QStringList cvLibNames = get_lib_names(libPath, "opencv_world");
    QStringList cvRelLibNames;
    QStringList cvDbgLibNames = get_lib_names(libPath, "d.lib");
    /* old version or self compiled */
    if (cvLibNames.isEmpty())
    {
        cvLibNames = get_lib_names(libPath);
    }

    for (auto libName : cvLibNames)
    {
        if (!cvDbgLibNames.contains(libName))
        {
            cvRelLibNames.push_back(libName);
        }
    }

    appendIncPath(rootDir + "/include", true);
    appendIncPath(rootDir + "/include", false);
    appendLibPath(libPath, true);
    appendLibPath(libPath, false);
    appendLibNames(cvDbgLibNames, true);
    appendLibNames(cvRelLibNames, false);
}

void library_auto_scan_widget::scan_boost(const QString &rootDir)
{
    /* installed using the official installer */
    if (is_file_or_dir_exists(rootDir + "/boost"))
    {
        appendIncPath(rootDir, true);
        appendIncPath(rootDir, false);

        QDir libDir(rootDir);
        QStringList subPaths = libDir.entryList();
        /* get lib path */
        QString libPath;
        for (auto dir : subPaths)
        {
            if (dir.contains("lib64-msvc-"))
            {
                libPath = rootDir + "/" + dir;
                break;
            }
            else if (dir.contains("lib86-msvc-"))
            {
                libPath = rootDir + "/" + dir;
                break;
            }
        }

        appendLibPath(libPath, true);
        appendLibPath(libPath, false);

        appendLibNames(
            get_lib_names(libPath,
                          "-mt-gd-" + ui->platform_combo->currentText()),
            true);
        appendLibNames(
            get_lib_names(libPath, "-mt-" + ui->platform_combo->currentText()),
            false);
    }
    /* self compile or compiled by vcpkg */
    else if (is_file_or_dir_exists(rootDir + "/include"))
    {
        appendIncPath(get_specific_inc_path(rootDir, "boost"), true);
        appendIncPath(get_specific_inc_path(rootDir, "boost"), false);

        QString libPath = get_specific_lib_path(rootDir);
        appendLibPath(libPath, true);
        appendLibPath(libPath, false);

        QString boostDbgLibSuffix =
            "-mt-gd-x64-" + get_specific_version(rootDir, "boost") + ".lib";
        QString boostRelLibSuffix =
            "-mt-x64-" + get_specific_version(rootDir, "boost") + ".lib";

        QStringList boostDbgLibNames = get_lib_names(libPath, "-mt-gd.lib");
        QStringList boostRelLibNames = get_lib_names(libPath, "-mt.lib");
        if (boostDbgLibNames.isEmpty())
        {
            boostDbgLibNames = get_lib_names(libPath, boostDbgLibSuffix);
        }
        if (boostRelLibNames.isEmpty())
        {
            boostRelLibNames = get_lib_names(libPath, boostRelLibSuffix);
        }

        appendLibNames(boostDbgLibNames, true);
        appendLibNames(boostRelLibNames, false);
    }
}

void library_auto_scan_widget::scan_vtk(const QString &rootDir)
{
    appendIncPath(get_specific_inc_path(rootDir, "vtk"), true);
    appendIncPath(get_specific_inc_path(rootDir, "vtk"), false);

    QString libPath = get_specific_lib_path(rootDir);
    appendLibPath(libPath, true);
    appendLibPath(libPath, false);

    /* other lib's 3rdParty */
    QString vtkDbgLibSuffix1 =
        "-" + get_specific_version(rootDir, "vtk") + "-gd.lib";
    /* vcpkg compiled */
    QString vtkDbgLibSuffix2 =
        "-" + get_specific_version(rootDir, "vtk") + "d.lib";
    /* release */
    QString vtkRelLibSuffix =
        "-" + get_specific_version(rootDir, "vtk") + ".lib";

    QStringList vtkDbgLibNames;
    QStringList vtkRelLibNames;
    if (is_file_or_dir_exists(rootDir + "/lib/vtkIOCore" + vtkDbgLibSuffix1))
    {
        vtkDbgLibNames = get_lib_names(libPath, vtkDbgLibSuffix1);
    }
    else if (is_file_or_dir_exists(rootDir + "/lib/vtkIOCore" +
                                   vtkDbgLibSuffix2))
    {
        vtkDbgLibNames = get_lib_names(libPath, vtkDbgLibSuffix2);
    }

    if (is_file_or_dir_exists(rootDir + "/lib/vtkIOCore" + vtkRelLibSuffix))
    {
        vtkRelLibNames = get_lib_names(libPath, vtkRelLibSuffix);
        if (vtkDbgLibNames.isEmpty())
        {
            vtkDbgLibNames = vtkRelLibNames;
        }
    }

    appendLibNames(vtkDbgLibNames, true);
    appendLibNames(vtkRelLibNames, false);
}

void library_auto_scan_widget::scan_pcl(const QString &rootDir)
{
    /* 3rdParty -> Boost、VTK、FLANN、Qhull、OpenNI2、Eigen */

    /* inc path */
    QStringList incPaths;
    QStringList incPathsPossible = {
        get_specific_inc_path(rootDir, "pcl"),
        get_specific_inc_path(rootDir + "/3rdParty/Boost", "boost"),
        get_specific_inc_path(rootDir + "/3rdParty/VTK", "vtk"),
        rootDir + "/3rdParty/FLANN/include",
        rootDir + "/3rdParty/Qhull/include"};

    for (auto path : incPathsPossible)
    {
        if (!path.isEmpty() && is_file_or_dir_exists(path))
        {
            incPaths.push_back(path);
        }
    }

    /* read openni2 inc path from env */
    QString openNI2IncPath = getenvSafe("OPENNI2_INCLUDE64");
    if (openNI2IncPath.isEmpty())
    {
        openNI2IncPath = rootDir + "/3rdParty/OpenNI2/Include";
    }

    /* read openni2 lib path from env */
    QString openNI2LibPath = getenvSafe("OPENNI2_LIB64");
    if (openNI2LibPath.isEmpty())
    {
        openNI2LibPath = rootDir + "/3rdParty/OpenNI2/Lib";
    }

    /* Eigen3 */
    if (is_file_or_dir_exists(rootDir + "/3rdParty/Eigen3/include/eigen3"))
    {
        incPaths.push_back(rootDir + "/3rdParty/Eigen3/include/eigen3");
    }
    else if (is_file_or_dir_exists(rootDir + "/3rdParty/Eigen3/eigen3"))
    {
        incPaths.push_back(rootDir + "/3rdParty/Eigen3/eigen3");
    }

    /* OpenNI2 inc */
    incPaths.push_back(openNI2IncPath);

    /* lib path */
    QStringList libPaths;
    QStringList libPathsPossible = {
        rootDir + "/lib", rootDir + "/3rdParty/Boost/lib",
        rootDir + "/3rdParty/VTK/lib", rootDir + "/3rdParty/FLANN/lib",
        rootDir + "/3rdParty/Qhull/lib"};

    for (auto path : libPathsPossible)
    {
        if (!path.isEmpty() && is_file_or_dir_exists(path))
        {
            libPaths.push_back(path);
        }
    }

    libPaths.push_back(openNI2LibPath);

    /* libs */
    QStringList libDbgNames;
    QStringList libRelNames;
    QStringList pclLibNames = {
        "common",       "features",         "filters",   "io",
        "io_ply",       "kdtree",           "keypoints", "ml",
        "octree",       "outofcore",        "people",    "recognition",
        "registration", "sample_consensus", "search",    "segmentation",
        "stereo",       "surface",          "tracking",  "visualization"};

    for (int i = 0; i < pclLibNames.size(); i++)
    {
        if (is_file_or_dir_exists(rootDir + "/lib/pcl_" + pclLibNames.at(i) +
                                  ".lib"))
        {
            libDbgNames.push_back("pcl_" + pclLibNames.at(i) + "d.lib");
            libRelNames.push_back("pcl_" + pclLibNames.at(i) + ".lib");
        }
        else if (is_file_or_dir_exists(rootDir + "/lib/pcl_" +
                                       pclLibNames.at(i) + "_release.lib"))
        {
            libDbgNames.push_back("pcl_" + pclLibNames.at(i) + "_debug.lib");
            libRelNames.push_back("pcl_" + pclLibNames.at(i) + "_release.lib");
        }
    }

    /* Boost */
    QString boostDbgLibSuffix =
        "-mt-gd-x64-" +
        get_specific_version(rootDir + "/3rdParty/Boost", "boost") + ".lib";
    QString boostRelLibSuffix =
        "-mt-x64-" +
        get_specific_version(rootDir + "/3rdParty/Boost", "boost") + ".lib";

    QStringList boostDbgLibNames =
        get_lib_names(rootDir + "/3rdParty/Boost/lib", boostDbgLibSuffix);
    QStringList boostRelLibNames =
        get_lib_names(rootDir + "/3rdParty/Boost/lib", boostRelLibSuffix);

    libDbgNames.append(boostDbgLibNames);
    libRelNames.append(boostRelLibNames);

    /* VTK */
    QString vtkDbgLibSuffix =
        "-" + get_specific_version(rootDir + "/3rdParty/VTK", "vtk") +
        "-gd.lib";
    QString vtkRelLibSuffix =
        "-" + get_specific_version(rootDir + "/3rdParty/VTK", "vtk") + ".lib";

    libDbgNames.append(
        get_lib_names(rootDir + "/3rdParty/VTK/lib", vtkDbgLibSuffix));
    libRelNames.append(
        get_lib_names(rootDir + "/3rdParty/VTK/lib", vtkRelLibSuffix));

    /* flann */
    if (is_file_or_dir_exists(rootDir + "/3rdParty/FLANN/lib/flann-gd.lib"))
    {
        libDbgNames.push_back("flann-gd.lib");
    }
    if (is_file_or_dir_exists(rootDir + "/3rdParty/FLANN/lib/flann_cpp-gd.lib"))
    {
        libDbgNames.push_back("flann_cpp-gd.lib");
    }
    if (is_file_or_dir_exists(rootDir + "/3rdParty/FLANN/lib/flann.lib"))
    {
        libRelNames.push_back("flann.lib");
    }
    if (is_file_or_dir_exists(rootDir + "/3rdParty/FLANN/lib/flann_cpp.lib"))
    {
        libRelNames.push_back("flann_cpp.lib");
    }

    /* qhull */
    if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhull.lib"))
    {
        libDbgNames.push_back("qhull_d.lib");
        libRelNames.push_back("qhull.lib");
    }

    if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhullcpp.lib"))
    {
        libDbgNames.push_back("qhullcpp_d.lib");
        libRelNames.push_back("qhullcpp.lib");
    }

    if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhull_r.lib"))
    {
        if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhull_rd.lib"))
        {
            libDbgNames.push_back("qhull_rd.lib");
        }
        else if (is_file_or_dir_exists(rootDir +
                                       "/3rdParty/Qhull/lib/qhull_r_d.lib"))
        {
            libDbgNames.push_back("qhull_r_d.lib");
        }

        libRelNames.push_back("qhull_r.lib");
    }

    if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhull_p.lib"))
    {
        libDbgNames.push_back("qhull_p_d.lib");
        libRelNames.push_back("qhull_p.lib");
    }

    /* OpenNI2 lib */
    if (is_file_or_dir_exists(openNI2LibPath + "/OpenNI2.lib"))
    {
        libDbgNames.push_back("OpenNI2.lib");
        libRelNames.push_back("OpenNI2.lib");
    }

    /* show incs、lib_dirs、libs */
    appendIncPaths(incPaths, true);
    appendIncPaths(incPaths, false);
    appendLibPaths(libPaths, true);
    appendLibPaths(libPaths, false);
    appendLibNames(libDbgNames, true);
    appendLibNames(libRelNames, false);
}

void library_auto_scan_widget::scan_lib_in_vcpkg_installed(
    const QString &vcpkgRootDir, const QString &triplet)
{
    scan_lib_in_vcpkg_installed(vcpkgRootDir + "/installed/" + triplet);
}

void library_auto_scan_widget::scan_lib_in_vcpkg_installed(
    const QString &rootDir)
{
    appendIncPath(rootDir + "/include", true);
    appendIncPath(rootDir + "/include", false);
    appendLibPath(rootDir + "/debug/lib", true);
    appendLibPath(rootDir + "/lib", false);
    appendLibNames(get_lib_names(rootDir + "/debug/lib"), true);
    appendLibNames(get_lib_names(rootDir + "/lib"), false);
}

QString library_auto_scan_widget::get_specific_inc_path(const QString &rootDir,
                                                        const QString &libName)
{
    /* get inc path */
    QString path;
    if (!libName.isEmpty())
    {
        if (!is_file_or_dir_exists(rootDir + "/include"))
        {
            return "";
        }

        QDir libDir(rootDir + "/include");
        QStringList subPaths = libDir.entryList();

        for (auto dir : subPaths)
        {
            if (dir.contains(libName))
            {
                path = rootDir + "/include/" + dir;
                break;
            }
        }
    }
    else
    {
        QStringList standard_incs_path = {"include", "Include", "inc"};

        for (auto inc : standard_incs_path)
        {
            if (is_file_or_dir_exists(rootDir + "/" + inc))
            {
                path = rootDir + "/" + inc;
                break;
            }
        }
    }

    return path;
}

QString library_auto_scan_widget::get_specific_version(const QString &rootDir,
                                                       const QString &libName)
{
    QString version;
    if (!is_file_or_dir_exists(rootDir + "/include"))
    {
        return "";
    }

    QDir libDir(rootDir + "/include");
    QStringList subPaths = libDir.entryList();

    for (auto dir : subPaths)
    {
        if (dir.contains(libName))
        {
            int i = dir.indexOf("-");
            if (i != -1)
            {
                QString s;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                s.assign(dir.begin() + i + 1, dir.end());
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                s = dir.right(dir.end() - dir.begin() - i - 1);
#endif
                version = s;
                break;
            }
        }
    }

    return version;
}

QString library_auto_scan_widget::get_specific_lib_path(const QString &rootDir)
{
    /* get lib path */
    QString path;

    if (is_file_or_dir_exists(rootDir + "/Lib/" +
                              ui->platform_combo->currentText()))
    {
        path = rootDir + "/Lib/" + ui->platform_combo->currentText();
    }
    else if (is_file_or_dir_exists(rootDir + "/lib"))
    {
        path = rootDir + "/lib";
    }

    return path;
}

QStringList library_auto_scan_widget::get_lib_names(const QString &libPath,
                                                    const QString &filter)
{
    QStringList libs;
    QDir libDir(libPath);
    auto eil = libDir.entryInfoList();

    for (auto e : eil)
    {
        if (!e.isDir() && e.fileName().contains(".lib") &&
            e.fileName().contains(filter))
        {
            libs.push_back(e.fileName());
        }
    }

    return libs;
}
