#include "library_auto_scan_widget.h"
#include "./ui_library_auto_scan_widget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <filesystem>

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

void library_auto_scan_widget::clear()
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

    connect(ui->ok_btn, &QPushButton::clicked, this,
            &library_auto_scan_widget::scan_complete);

    QStringList libNames = {"常规库[可叠加]",
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
    return std::filesystem::exists(path.toStdString());
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

    ui->incs_edit_dbg->setText(ui->incs_edit_dbg->toPlainText() + "\n" +
                               get_specific_inc_path(rootDir));
    ui->incs_edit_rel->setText(ui->incs_edit_dbg->toPlainText());

    QString libPath = get_specific_lib_path(rootDir);
    if (!libPath.isEmpty())
    {
        QString libsText = get_lib_names(libPath).join("\n");
        ui->lib_dir_edit_dbg->setText(ui->lib_dir_edit_dbg->toPlainText() +
                                      "\n" + libPath);
        ui->lib_dir_edit_rel->setText(ui->lib_dir_edit_dbg->toPlainText());

        ui->libs_edit_dbg->setText(ui->libs_edit_dbg->toPlainText() + "\n" +
                                   libsText);
        ui->libs_edit_rel->setText(ui->libs_edit_rel->toPlainText() + "\n" +
                                   libsText);
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

    ui->incs_edit_dbg->setText(rootDir + "/include");
    ui->incs_edit_rel->setText(rootDir + "/include");
    ui->lib_dir_edit_dbg->setText(libPath);
    ui->lib_dir_edit_rel->setText(libPath);
    ui->libs_edit_dbg->setText(cvDbgLibNames.join("\n"));
    ui->libs_edit_rel->setText(cvRelLibNames.join("\n"));
}

void library_auto_scan_widget::scan_boost(const QString &rootDir)
{
    /* installed using the official installer */
    if (is_file_or_dir_exists(rootDir + "/boost"))
    {
        ui->incs_edit_dbg->setText(rootDir);
        ui->incs_edit_rel->setText(rootDir);

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

        ui->lib_dir_edit_dbg->setText(libPath);
        ui->lib_dir_edit_rel->setText(libPath);

        ui->libs_edit_dbg->setText(
            get_lib_names(libPath,
                          "-mt-gd-" + ui->platform_combo->currentText())
                .join("\n"));
        ui->libs_edit_rel->setText(
            get_lib_names(libPath, "-mt-" + ui->platform_combo->currentText())
                .join("\n"));
    }
    /* self compile or compiled by vcpkg */
    else if (is_file_or_dir_exists(rootDir + "/include"))
    {
        ui->incs_edit_dbg->setText(get_specific_inc_path(rootDir, "boost"));
        ui->incs_edit_rel->setText(ui->incs_edit_dbg->toPlainText());

        QString libPath = get_specific_lib_path(rootDir);
        ui->lib_dir_edit_dbg->setText(libPath);
        ui->lib_dir_edit_rel->setText(libPath);

        ui->libs_edit_dbg->setText(
            get_lib_names(libPath, "-mt-gd.lib").join("\n"));
        ui->libs_edit_rel->setText(
            get_lib_names(libPath, "-mt.lib").join("\n"));
    }
}

void library_auto_scan_widget::scan_vtk(const QString &rootDir)
{
    ui->incs_edit_dbg->setText(get_specific_inc_path(rootDir, "vtk"));
    ui->incs_edit_rel->setText(ui->incs_edit_dbg->toPlainText());

    QString libPath = get_specific_lib_path(rootDir);
    ui->lib_dir_edit_dbg->setText(libPath);
    ui->lib_dir_edit_rel->setText(libPath);

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

    ui->libs_edit_dbg->setText(vtkDbgLibNames.join("\n"));
    ui->libs_edit_rel->setText(vtkRelLibNames.join("\n"));
}

void library_auto_scan_widget::scan_pcl(const QString &rootDir)
{
    /* 3rdParty -> Boost、VTK、FLANN、Qhull、OpenNI2、Eigen */

    /* inc path */
    QStringList incPaths = {
        get_specific_inc_path(rootDir, "pcl"),
        get_specific_inc_path(rootDir + "/3rdParty/Boost", "boost"),
        get_specific_inc_path(rootDir + "/3rdParty/VTK", "vtk"),
        rootDir + "/3rdParty/FLANN/include",
        rootDir + "/3rdParty/Qhull/include"};

    /* read openni2 inc path from env */
    QString openNI2IncPath = std::getenv("OPENNI2_INCLUDE64");
    openNI2IncPath.replace("\\", "/");
    if (openNI2IncPath.at(openNI2IncPath.size() - 1) == "/")
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        openNI2IncPath.removeAt(openNI2IncPath.size() - 1);
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        openNI2IncPath.remove(openNI2IncPath.size() - 1, 1);
#endif
    }

    if (openNI2IncPath.isEmpty())
    {
        openNI2IncPath = rootDir + "/3rdParty/OpenNI2/Include";
    }

    /* read openni2 lib path from env */
    QString openNI2LibPath = std::getenv("OPENNI2_LIB64");
    openNI2LibPath.replace("\\", "/");
    if (openNI2LibPath.at(openNI2LibPath.size() - 1) == "/")
    {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        openNI2LibPath.removeAt(openNI2LibPath.size() - 1);
#elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        openNI2LibPath.remove(openNI2LibPath.size() - 1, 1);
#endif
    }

    if (openNI2LibPath.isEmpty())
    {
        openNI2LibPath = rootDir + "/3rdParty/FLANN/Lib";
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
    QStringList libPaths = {rootDir + "/lib", rootDir + "/3rdParty/Boost/lib",
                            rootDir + "/3rdParty/VTK/lib",
                            rootDir + "/3rdParty/FLANN/lib",
                            rootDir + "/3rdParty/Qhull/lib"};

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
    libDbgNames.push_back("flann-gd.lib");
    libDbgNames.push_back("flann_cpp-gd.lib");
    libRelNames.push_back("flann.lib");
    libRelNames.push_back("flann_cpp.lib");

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
    libDbgNames.push_back("OpenNI2.lib");
    libRelNames.push_back("OpenNI2.lib");
    /* show incs、lib_dirs、libs */
    ui->incs_edit_dbg->setText(incPaths.join("\n"));
    ui->incs_edit_rel->setText(incPaths.join("\n"));
    ui->lib_dir_edit_dbg->setText(libPaths.join("\n"));
    ui->lib_dir_edit_rel->setText(libPaths.join("\n"));
    ui->libs_edit_dbg->setText(libDbgNames.join("\n"));
    ui->libs_edit_rel->setText(libRelNames.join("\n"));
}

void library_auto_scan_widget::scan_lib_in_vcpkg_installed(
    const QString &vcpkgRootDir, const QString &triplet)
{
    scan_lib_in_vcpkg_installed(vcpkgRootDir + "/installed/" + triplet);
}

void library_auto_scan_widget::scan_lib_in_vcpkg_installed(
    const QString &rootDir)
{
    ui->incs_edit_dbg->setText(ui->incs_edit_dbg->toPlainText() + "\n" +
                               rootDir + "/include");
    ui->incs_edit_rel->setText(ui->incs_edit_dbg->toPlainText());

    ui->lib_dir_edit_dbg->setText(ui->lib_dir_edit_dbg->toPlainText() + "\n" +
                                  rootDir + "/debug/lib");
    ui->lib_dir_edit_rel->setText(ui->lib_dir_edit_rel->toPlainText() + "\n" +
                                  rootDir + "/lib");

    ui->libs_edit_dbg->setText(
        ui->libs_edit_dbg->toPlainText() + "\n" +
        get_lib_names(rootDir + "/debug/lib").join("\n"));
    ui->libs_edit_rel->setText(ui->libs_edit_rel->toPlainText() + "\n" +
                               get_lib_names(rootDir + "/lib").join("\n"));
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
