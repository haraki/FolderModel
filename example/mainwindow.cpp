#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "foldermodel.h"

using namespace Farman;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_folderModel(new FolderModel(this))
    , m_rootPathChanging(false)
{
    ui->setupUi(this);

    initModel();

    ui->folderView->setModel(m_folderModel);
    ui->folderView->setRootIndex(m_folderModel->index(m_folderModel->rootPath()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initModel()
{
    m_folderModel->setFilterFlags(FilterFlag::AllEntrys | FilterFlag::Hidden | FilterFlag::System);
    m_folderModel->setNameFilters(ui->nameMaskFilterLineEdit->text().split(' '));

    if(ui->nameSortRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType(SectionType::FileName);
    }
    else if(ui->extSortRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType(SectionType::FileType);
    }
    else if(ui->sizeSortRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType(SectionType::FileSize);
    }
    else if(ui->lastModifiedSortRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType(SectionType::LastModified);
    }

    if(ui->nameSort2ndRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType2nd(SectionType::FileName);
    }
    else if(ui->extSort2ndRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType2nd(SectionType::FileType);
    }
    else if(ui->sizeSort2ndRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType2nd(SectionType::FileSize);
    }
    else if(ui->lastModifiedSort2ndRadioButton->isChecked())
    {
        m_folderModel->setSortSectionType2nd(SectionType::LastModified);
    }

    connect(m_folderModel, SIGNAL(rootPathChanged(const QString&)), this, SLOT(rootPathChanged(const QString&)));

    m_folderModel->setRootPath(QDir::homePath());

    m_rootPathChanging = true;
}

void MainWindow::rootPathChanged(const QString& path)
{
    ui->currentPathLineEdit->setText(path);

    ui->folderView->setRootIndex(m_folderModel->index(path));

    m_rootPathChanging = false;
}

void MainWindow::on_folderView_doubleClicked(const QModelIndex &index)
{
    if(!m_folderModel->isDir(index))
    {
        qDebug() << "Not directory.";
        return;
    }

    QString newPath = m_folderModel->fileInfo(index).absoluteFilePath();

    qDebug() << "newPath : " << newPath;

    m_rootPathChanging = true;

    if(m_folderModel->setRootPath(newPath) < 0)
    {
        m_rootPathChanging = false;
    }
}

void MainWindow::on_hiddenFilterCheckBox_clicked(bool checked)
{
    FilterFlags filterFlag = m_folderModel->filterFlags();
    if(checked)
    {
        filterFlag |= FilterFlag::Hidden;
    }
    else
    {
        filterFlag &= ~static_cast<int>(FilterFlag::Hidden);
    }

    m_folderModel->setFilterFlags(filterFlag);

    m_folderModel->refresh();
}

void MainWindow::on_systemFilterCheckBox_clicked(bool checked)
{
    FilterFlags filterFlag = m_folderModel->filterFlags();
    if(checked)
    {
        filterFlag |= FilterFlag::System;
    }
    else
    {
        filterFlag &= ~static_cast<int>(FilterFlag::System);
    }

    m_folderModel->setFilterFlags(filterFlag);

    m_folderModel->refresh();
}

void MainWindow::on_filesFilterCheckBox_clicked(bool checked)
{
    FilterFlags filterFlag = m_folderModel->filterFlags();
    if(checked)
    {
        filterFlag |= FilterFlag::Files;
    }
    else
    {
        filterFlag &= ~static_cast<int>(FilterFlag::Files);
    }

    m_folderModel->setFilterFlags(filterFlag);

    m_folderModel->refresh();
}

void MainWindow::on_dirsFilterCheckBox_clicked(bool checked)
{
    FilterFlags filterFlag = m_folderModel->filterFlags();
    if(checked)
    {
        filterFlag |= FilterFlag::Dirs;
    }
    else
    {
        filterFlag &= ~static_cast<int>(FilterFlag::Dirs);
    }

    m_folderModel->setFilterFlags(filterFlag);

    m_folderModel->refresh();
}

void MainWindow::on_nameMaskFilterLineEdit_textEdited(const QString &arg1)
{
    Q_UNUSED(arg1)

    QStringList nameFilters = ui->nameMaskFilterLineEdit->text().split(' ');
    m_folderModel->setNameFilters(nameFilters);

    m_folderModel->refresh();
}

void MainWindow::on_nameSortRadioButton_clicked()
{
    m_folderModel->setSortSectionType(SectionType::FileName);

    if(ui->nameSort2ndRadioButton->isChecked())
    {
        ui->noneSort2ndRadioButton->setChecked(true);

        m_folderModel->setSortSectionType2nd(SectionType::Unknown);
    }
    ui->nameSort2ndRadioButton->setEnabled(false);
    ui->extSort2ndRadioButton->setEnabled(true);
    ui->sizeSort2ndRadioButton->setEnabled(true);
    ui->lastModifiedSort2ndRadioButton->setEnabled(true);

    m_folderModel->refresh();
}

void MainWindow::on_extSortRadioButton_clicked()
{
    m_folderModel->setSortSectionType(SectionType::FileType);

    if(ui->extSort2ndRadioButton->isChecked())
    {
        ui->noneSort2ndRadioButton->setChecked(true);

        m_folderModel->setSortSectionType2nd(SectionType::Unknown);
    }
    ui->nameSort2ndRadioButton->setEnabled(true);
    ui->extSort2ndRadioButton->setEnabled(false);
    ui->sizeSort2ndRadioButton->setEnabled(true);
    ui->lastModifiedSort2ndRadioButton->setEnabled(true);

    m_folderModel->refresh();
}

void MainWindow::on_sizeSortRadioButton_clicked()
{
    m_folderModel->setSortSectionType(SectionType::FileSize);

    if(ui->sizeSort2ndRadioButton->isChecked())
    {
        ui->noneSort2ndRadioButton->setChecked(true);

        m_folderModel->setSortSectionType2nd(SectionType::Unknown);
    }
    ui->nameSort2ndRadioButton->setEnabled(true);
    ui->extSort2ndRadioButton->setEnabled(true);
    ui->sizeSort2ndRadioButton->setEnabled(false);
    ui->lastModifiedSort2ndRadioButton->setEnabled(true);

    m_folderModel->refresh();
}

void MainWindow::on_lastModifiedSortRadioButton_clicked()
{
    m_folderModel->setSortSectionType(SectionType::LastModified);

    if(ui->lastModifiedSort2ndRadioButton->isChecked())
    {
        ui->noneSort2ndRadioButton->setChecked(true);

        m_folderModel->setSortSectionType2nd(SectionType::Unknown);
    }
    ui->nameSort2ndRadioButton->setEnabled(true);
    ui->extSort2ndRadioButton->setEnabled(true);
    ui->sizeSort2ndRadioButton->setEnabled(true);
    ui->lastModifiedSort2ndRadioButton->setEnabled(false);

    m_folderModel->refresh();
}

void MainWindow::on_nameSort2ndRadioButton_clicked()
{
    m_folderModel->setSortSectionType2nd(SectionType::FileName);

    m_folderModel->refresh();
}

void MainWindow::on_extSort2ndRadioButton_clicked()
{
    m_folderModel->setSortSectionType2nd(SectionType::FileType);

    m_folderModel->refresh();
}

void MainWindow::on_sizeSort2ndRadioButton_clicked()
{
    m_folderModel->setSortSectionType2nd(SectionType::FileSize);

    m_folderModel->refresh();
}

void MainWindow::on_lastModifiedSort2ndRadioButton_clicked()
{
    m_folderModel->setSortSectionType2nd(SectionType::LastModified);

    m_folderModel->refresh();
}

void MainWindow::on_noneSort2ndRadioButton_clicked()
{
    m_folderModel->setSortSectionType2nd(SectionType::Unknown);

    m_folderModel->refresh();
}

void MainWindow::on_firstFoldersRadioButton_clicked()
{
    m_folderModel->setSortDirsType(SortDirsType::First);

    m_folderModel->refresh();
}

void MainWindow::on_lastFoldersRadioButton_clicked()
{
    m_folderModel->setSortDirsType(SortDirsType::Last);

    m_folderModel->refresh();
}

void MainWindow::on_NoSpecifyRadioButton_clicked()
{
    m_folderModel->setSortDirsType(SortDirsType::NoSpecify);

    m_folderModel->refresh();
}

void MainWindow::on_ascendingRadioButton_clicked()
{
    m_folderModel->setSortOrder(SortOrderType::Ascending);

    m_folderModel->refresh();
}

void MainWindow::on_decendingRadioButton_clicked()
{
    m_folderModel->setSortOrder(SortOrderType::Descending);

    m_folderModel->refresh();
}

void MainWindow::on_sensitiveRadioButton_clicked()
{
    m_folderModel->setSortCaseSensitivity(SortCaseSensitivity::Sensitive);

    m_folderModel->refresh();
}

void MainWindow::on_InsensitiveRadioButton_clicked()
{
    m_folderModel->setSortCaseSensitivity(SortCaseSensitivity::Insensitive);

    m_folderModel->refresh();
}

void MainWindow::on_formatFileSizeSIRadioButton_clicked()
{
    ui->formatFileSizeCommaCheckBox->setEnabled(false);

    m_folderModel->setFileSizeFormatType(FileSizeFormatType::SI);

    m_folderModel->refresh();
}

void MainWindow::on_formatFileSizeIecRadioButton_clicked()
{
    ui->formatFileSizeCommaCheckBox->setEnabled(false);

    m_folderModel->setFileSizeFormatType(FileSizeFormatType::IEC);

    m_folderModel->refresh();
}

void MainWindow::on_formatFileSizeDetailRadioButton_clicked()
{
    ui->formatFileSizeCommaCheckBox->setEnabled(true);

    m_folderModel->setFileSizeFormatType(FileSizeFormatType::Detail);

    m_folderModel->refresh();
}

void MainWindow::on_formatFileSizeCommaCheckBox_clicked(bool checked)
{
    m_folderModel->setFileSizeComma(checked);

    m_folderModel->refresh();
}

void MainWindow::on_formatDateDefaultRadioButton_clicked()
{
    ui->formatDateOriginalLineEdit->setEnabled(false);

    m_folderModel->setDateFormatType(DateFormatType::Default);

    m_folderModel->refresh();
}

void MainWindow::on_formatDateISORadioButton_clicked()
{
    ui->formatDateOriginalLineEdit->setEnabled(false);

    m_folderModel->setDateFormatType(DateFormatType::ISO);

    m_folderModel->refresh();
}

void MainWindow::on_formatDateOriginalRadioButton_clicked()
{
    ui->formatDateOriginalLineEdit->setEnabled(true);

    m_folderModel->setDateFormatType(DateFormatType::Original);

    m_folderModel->refresh();
}

void MainWindow::on_formatDateOriginalLineEdit_textEdited(const QString &arg1)
{
    m_folderModel->setDateFormatOriginalString(arg1);

    m_folderModel->refresh();
}
