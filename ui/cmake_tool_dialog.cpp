#include "cmake_tool_dialog.h"
#include "./ui_cmake_tool_dialog.h"

#include <QClipboard>
#include <QProcess>

cmake_tool_dialog::cmake_tool_dialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::cmake_tool_dialog)
{
    ui->setupUi(this);

    init();
}

cmake_tool_dialog::~cmake_tool_dialog() { delete ui; }

void cmake_tool_dialog::updateSourceDir(const QString &s)
{
    m_source_dir = s;
    m_source_dir = m_source_dir.replace("\\", "/");
    m_build_dir = m_source_dir + "/build";
    m_install_dir = m_source_dir + "/install";

    updateCMakeCommand();
}

void cmake_tool_dialog::updateBuildDir(const QString &s)
{
    m_build_dir = s;

    updateCMakeCommand();
}

void cmake_tool_dialog::updateInstallDir(const QString &s)
{
    m_install_dir = s;

    updateCMakeCommand();
}

void cmake_tool_dialog::updateCMakeSelector()
{
    QProcess process(this);
    process.setProgram("C:/Windows/System32/where.exe");
    process.setArguments(QStringList() << "cmake");
    process.start();
    process.waitForStarted();
    process.waitForFinished();
    QString cmakePaths =
        QString::fromLocal8Bit(process.readAllStandardOutput());
    ui->cmake_combo->clear();
    ui->cmake_combo->addItems(cmakePaths.split("\n"));
}

void cmake_tool_dialog::updateCMakeArgs()
{
    const QString &s = ui->args_edit->toPlainText();

    m_args_list.clear();
    QStringList lines = s.split("\n");

    for (auto line : lines)
    {
        QStringList l = line.split("=");
        if (l.size() > 1)
        {
            m_args_list.push_back(
                QPair<QString, QString>(l[0], l[1].replace("\\", "/")));
        }
    }

    updateCMakeCommand();
}
void cmake_tool_dialog::updateCMakeCommand()
{
    if (m_source_dir.isEmpty())
    {
        return;
    }

    QString cmakePath;
    QString currCMakePath = ui->cmake_combo->currentText();
    if (currCMakePath.isEmpty() || !currCMakePath.contains("cmake"))
    {
        cmakePath = "cmake";
    }
    else
    {
        if (currCMakePath.contains(" "))
        {
            cmakePath = ".\"" + currCMakePath + "\"";
        }
        else
        {
            cmakePath = currCMakePath;
        }
    }

    QString args;
    for (auto arg : m_args_list)
    {
        args += "-D" + arg.first + "=" + arg.second + " ";
    }

    QString cmakeCommand =
        cmakePath + " -S " + m_source_dir + " -B " + m_build_dir +
        "-DCMAKE_INSTALL_PREFIX=" + m_install_dir + " " + args;

    ui->content_edit->setText(cmakeCommand);
}

void cmake_tool_dialog::exit()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->content_edit->toPlainText());

    this->close();
}

void cmake_tool_dialog::init()
{
    connect(ui->cmake_combo, &QComboBox::currentTextChanged, this,
            &cmake_tool_dialog::updateCMakeCommand);
    connect(ui->src_dir_edit, &QLineEdit::textChanged, this,
            &cmake_tool_dialog::updateSourceDir);
    connect(ui->build_dir_edit, &QLineEdit::textChanged, this,
            &cmake_tool_dialog::updateBuildDir);
    connect(ui->install_dir_edit, &QLineEdit::textChanged, this,
            &cmake_tool_dialog::updateInstallDir);
    connect(ui->args_edit, &QTextEdit::textChanged, this,
            &cmake_tool_dialog::updateCMakeArgs);
    connect(ui->exit_btn, &QPushButton::clicked, this,
            &cmake_tool_dialog::exit);

    updateCMakeSelector();
}
