#include "file_selector_dialog.h"
#include "./ui_file_selector_dialog.h"

#include <QClipboard>
#include <QFileDialog>
#include <QListView>
#include <QMessageBox>
#include <QTreeView>

file_selector_dialog::file_selector_dialog(
    const file_selector_dialog_mode &mode, QWidget *parent)
    : QDialog(parent), ui(new Ui::file_selector_dialog), m_mode_private(mode)
{
    ui->setupUi(this);

    init();
}

file_selector_dialog::~file_selector_dialog() { delete ui; }

void file_selector_dialog::init()
{
    connect(ui->slash_combo, &QComboBox::currentTextChanged, this,
            &file_selector_dialog::changeSlashText);

    connect(ui->view_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &file_selector_dialog::switchView);

    connect(ui->replace_btn, &QPushButton::clicked, this,
            &file_selector_dialog::replaceAllText);

    connect(ui->single_btn, &QPushButton::clicked, this,
            &file_selector_dialog::single_select);

    connect(ui->multi_btn, &QPushButton::clicked, this,
            &file_selector_dialog::multi_select);

    connect(ui->exit_btn, &QPushButton::clicked, this,
            &file_selector_dialog::exit);

    ui->slash_combo->addItems(QStringList() << "\\" << "/" << "\\\\");
    ui->view_combo->addItems(QStringList() << "单行视图" << "多行视图");
    ui->view_combo->setCurrentIndex(1);
}

QStringList file_selector_dialog::getExistingDirectories(const QString &title,
                                                         const QString &path)
{
    QStringList folders;

    QFileDialog fileDlg(this, title, path);
    fileDlg.setFileMode(QFileDialog::Directory);
    fileDlg.setOption(QFileDialog::DontUseNativeDialog, true);
    QListView *listView = fileDlg.findChild<QListView *>("listView");
    if (listView)
        listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QTreeView *treeView = fileDlg.findChild<QTreeView *>();
    if (treeView)
        treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    if (fileDlg.exec())
    {
        folders = fileDlg.selectedFiles();
    }

    return folders;
}

void file_selector_dialog::changeSlashText(const QString &slash)
{
    m_slash_text = slash;
}

void file_selector_dialog::switchView(int i)
{
    switch (i)
    {
    case 0:
        /* single line */
        ui->content_edit->setText(
            ui->content_edit->toPlainText().replace("\n", ";"));
        break;

    case 1:
        /* multi lines */
        ui->content_edit->setText(
            ui->content_edit->toPlainText().replace(";", "\n"));
        break;

    default:
        break;
    }
}

void file_selector_dialog::replaceAllText()
{
    QString replaced = ui->replaced_edit->text();
    QString replace = ui->replace_edit->text();
    QString currText = ui->content_edit->toPlainText();
    if (replaced.isEmpty())
    {
        QMessageBox::warning(this, "警告", "被替换文本不能为空");
        return;
    }

    currText.replace(replaced, replace);
    ui->content_edit->setText(currText);
}

void file_selector_dialog::single_select()
{
    QString urlSelected;
    switch (m_mode_private)
    {
    case file_selector_dialog_mode::FS_FILE:
        urlSelected = QFileDialog::getOpenFileName(
            this, QStringLiteral("选择文件"), "", QStringLiteral("file(*.*)"));
        break;

    case file_selector_dialog_mode::FS_DIRECTORY:
        urlSelected = QFileDialog::getExistingDirectory(
            this, QStringLiteral("选择文件夹"), "");
        break;

    default:
        break;
    }

    if (!urlSelected.isEmpty())
    {
        QString orginalText = ui->content_edit->toPlainText();
        if (!orginalText.isEmpty())
        {
            orginalText += "\n";
        }

        urlSelected += ui->suffix_edit->text();
        urlSelected.replace("/", m_slash_text);
        ui->content_edit->setText(orginalText + urlSelected);
    }
}

void file_selector_dialog::multi_select()
{
    QStringList urlsSelected;
    switch (m_mode_private)
    {
    case file_selector_dialog_mode::FS_FILE:
        urlsSelected = QFileDialog::getOpenFileNames(
            this, QStringLiteral("选择文件"), "", QStringLiteral("file(*.*)"));
        break;

    case file_selector_dialog_mode::FS_DIRECTORY:
        urlsSelected =
            getExistingDirectories("选择文件夹", QDir::currentPath());
        break;

    default:
        break;
    }

    if (!urlsSelected.isEmpty())
    {
        QString orginalText = ui->content_edit->toPlainText();
        QString suffix = ui->suffix_edit->text();
        if (!orginalText.isEmpty())
        {
            orginalText += "\n";
        }

        for (auto &url : urlsSelected)
        {
            url += suffix;
            url.replace("/", m_slash_text);
        }

        ui->content_edit->setText(orginalText + urlsSelected.join("\n"));
    }
}

void file_selector_dialog::exit()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->content_edit->toPlainText());

    this->close();
}
