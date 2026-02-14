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

QStringList library_auto_scan_widget::get_inc_paths()
{
    QStringList paths = ui->incs_edit->toPlainText().split("\n");
    for (auto path : paths)
    {
        if (path.isEmpty())
        {
            paths.removeOne(path);
        }
    }

    return paths;
}

QStringList library_auto_scan_widget::get_lib_paths()
{
    QStringList paths = ui->lib_dir_edit->toPlainText().split("\n");
    for (auto path : paths)
    {
        if (path.isEmpty())
        {
            paths.removeOne(path);
        }
    }

    return paths;
}

QStringList library_auto_scan_widget::get_lib_names()
{
    QStringList names = ui->libs_edit->toPlainText().split("\n");
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
    if (ui->incs_edit->toPlainText().isEmpty() &&
        ui->lib_dir_edit->toPlainText().isEmpty() &&
        ui->libs_edit->toPlainText().isEmpty())
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

    ui->incs_edit->clear();
    ui->lib_dir_edit->clear();
    ui->libs_edit->clear();
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
        /* Boost */
        scan_boost(rootDir);
        break;

    case 2:
        /* PCL */
        scan_pcl(rootDir);
        break;

    case 3:
        /* vcpkg */
        scan_lib_in_vcpkg_installed(rootDir, ui->platform_combo->currentText() +
                                                 "-windows");
        break;

    case 4:
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

    QStringList libNames = {"常规库[可叠加]", "Boost", "PCL", "vcpkg : windows",
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
    ui->incs_edit->setText(ui->incs_edit->toPlainText() + "\n" +
                           get_specific_inc_path(rootDir));

    QString libPath = get_specific_lib_path(rootDir);
    if (!libPath.isEmpty())
    {
        ui->lib_dir_edit->setText(ui->lib_dir_edit->toPlainText() + "\n" +
                                  libPath);
        ui->libs_edit->setText(ui->libs_edit->toPlainText() + "\n" +
                               get_lib_names(libPath).join("\n"));
    }
}

void library_auto_scan_widget::scan_boost(const QString &rootDir)
{
    /* installed using the official installer */
    if (is_file_or_dir_exists(rootDir + "/boost"))
    {
        ui->incs_edit->setText(rootDir);

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

        ui->incs_edit->setText(rootDir + "/boost");
        ui->lib_dir_edit->setText(libPath);
        ui->libs_edit->setText(
            get_lib_names(libPath, "-mt-" + ui->platform_combo->currentText())
                .join("\n"));
    }
    /* self compile or compiled by vcpkg */
    else if (is_file_or_dir_exists(rootDir + "/include"))
    {
        ui->incs_edit->setText(get_specific_inc_path(rootDir, "boost"));

        QString libPath = get_specific_lib_path(rootDir);
        ui->lib_dir_edit->setText(libPath);
        ui->libs_edit->setText(get_lib_names(libPath, "-mt.lib").join("\n"));
    }
}

void library_auto_scan_widget::scan_vtk(const QString &rootDir)
{
    ui->incs_edit->setText(get_specific_inc_path(rootDir, "vtk"));
    QString libPath = get_specific_lib_path(rootDir);
    ui->lib_dir_edit->setText(libPath);
    ui->libs_edit->setText(get_lib_names(libPath).join("\n"));
}

void library_auto_scan_widget::scan_pcl(const QString &rootDir)
{
    /* 3rdParty -> Boost、VT、FLANN、Qhull、OpenNI2、Eigen */

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
        openNI2IncPath.removeAt(openNI2IncPath.size() - 1);
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
        openNI2LibPath.removeAt(openNI2LibPath.size() - 1);
    }

    if (openNI2LibPath.isEmpty())
    {
        openNI2LibPath = rootDir + "/3rdParty/FLANN/Lib";
    }

    incPaths.push_back(openNI2IncPath);

    /* Eigen3 */
    if (is_file_or_dir_exists(rootDir + "/3rdParty/Eigen3/include/eigen3"))
    {
        incPaths.push_back(rootDir + "/3rdParty/Eigen3/include/eigen3");
    }
    else if (is_file_or_dir_exists(rootDir + "/3rdParty/Eigen3/eigen3"))
    {
        incPaths.push_back(rootDir + "/3rdParty/Eigen3/eigen3");
    }

    /* lib path */
    QStringList libPaths = {rootDir + "/lib", rootDir + "/3rdParty/Boost/lib",
                            rootDir + "/3rdParty/VTK/lib",
                            rootDir + "/3rdParty/FLANN/lib",
                            rootDir + "/3rdParty/Qhull/lib"};

    libPaths.push_back(openNI2LibPath);

    /* libs */
    QStringList libNames;
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
            libNames.push_back("pcl_" + pclLibNames.at(i) + ".lib");
        }
        else if (is_file_or_dir_exists(rootDir + "/lib/pcl_" +
                                       pclLibNames.at(i) + "_release.lib"))
        {
            libNames.push_back("pcl_" + pclLibNames.at(i) + "_release.lib");
        }
    }

    /* Boost */
    QString boostLibSuffix =
        "-mt-" + get_specific_version(rootDir + "/3rdParty/Boost", "boost") +
        ".lib";
    QStringList boostLibNames =
        get_lib_names(rootDir + "/3rdParty/Boost/lib",
                      "-mt-" + ui->platform_combo->currentText());

    if (boostLibNames.isEmpty())
    {
        boostLibNames =
            get_lib_names(rootDir + "/3rdParty/Boost/lib", boostLibSuffix);
    }

    libNames.append(boostLibNames);
    /* VTK */
    QString vtkLibSuffix =
        "-" + get_specific_version(rootDir + "/3rdParty/VTK", "vtk") + ".lib";
    QMessageBox::warning(this, "", vtkLibSuffix);
    libNames.append(get_lib_names(rootDir + "/3rdParty/VTK/lib", vtkLibSuffix));

    /* flann */
    libNames.push_back("flann.lib");
    libNames.push_back("flann_cpp.lib");
    /* qhull */
    libNames.push_back("qhull.lib");
    libNames.push_back("qhull_cpp.lib");
    libNames.push_back("qhull_r.lib");
    if (is_file_or_dir_exists(rootDir + "/3rdParty/Qhull/lib/qhull_p.lib"))
    {
        libNames.push_back("qhull_p.lib");
    }
    /* openni2 */
    libNames.push_back("OpenNI2.lib");

    ui->incs_edit->setText(incPaths.join("\n"));
    ui->lib_dir_edit->setText(libPaths.join("\n"));
    ui->libs_edit->setText(libNames.join("\n"));
}

void library_auto_scan_widget::scan_lib_in_vcpkg_installed(
    const QString &vcpkgRootDir, const QString &triplet)
{
    ui->incs_edit->setText(vcpkgRootDir + "/installed/" + triplet + "/include");
    ui->lib_dir_edit->setText(vcpkgRootDir + "/installed/" + triplet + "/lib");
    ui->libs_edit->setText(
        get_lib_names(vcpkgRootDir + "/installed/" + triplet + "/lib")
            .join("\n"));
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
                s.assign(dir.begin() + i + 1, dir.end());
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
