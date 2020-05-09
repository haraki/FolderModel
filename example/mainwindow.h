#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
namespace Farman {
    class FolderModel;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private Q_SLOTS:
    void rootPathChanged(const QString& path);

    void on_folderView_doubleClicked(const QModelIndex &index);

    void on_hiddenFilterCheckBox_clicked(bool checked);
    void on_systemFilterCheckBox_clicked(bool checked);
    void on_filesFilterCheckBox_clicked(bool checked);
    void on_dirsFilterCheckBox_clicked(bool checked);
    void on_nameMaskFilterLineEdit_textEdited(const QString &arg1);

    void on_nameSortRadioButton_clicked();
    void on_extSortRadioButton_clicked();
    void on_sizeSortRadioButton_clicked();
    void on_lastModifiedSortRadioButton_clicked();
    void on_nameSort2ndRadioButton_clicked();
    void on_extSort2ndRadioButton_clicked();
    void on_sizeSort2ndRadioButton_clicked();
    void on_lastModifiedSort2ndRadioButton_clicked();
    void on_noneSort2ndRadioButton_clicked();
    void on_firstFoldersRadioButton_clicked();
    void on_lastFoldersRadioButton_clicked();
    void on_NoSpecifyRadioButton_clicked();
    void on_ascendingRadioButton_clicked();
    void on_decendingRadioButton_clicked();
    void on_sensitiveRadioButton_clicked();
    void on_InsensitiveRadioButton_clicked();

    void on_formatFileSizeSIRadioButton_clicked();
    void on_formatFileSizeIecRadioButton_clicked();
    void on_formatFileSizeDetailRadioButton_clicked();
    void on_formatFileSizeCommaCheckBox_clicked(bool checked);
    void on_formatDateDefaultRadioButton_clicked();
    void on_formatDateISORadioButton_clicked();
    void on_formatDateOriginalRadioButton_clicked();
    void on_formatDateOriginalLineEdit_textEdited(const QString &arg1);

private:
    void initModel();

    Ui::MainWindow *ui;

    Farman::FolderModel* m_folderModel;

    bool m_rootPathChanging;
};
#endif // MAINWINDOW_H
