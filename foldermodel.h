#ifndef DIRMODEL_H
#define DIRMODEL_H

#include <QAbstractTableModel>
#include <QItemSelectionModel>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFont>

namespace Farman
{

enum class SectionType : int
{
    Unknown = -1,

    FileName = 0,
    FileType,
    FileSize,
    Owner,
    Group,
    Permissions,
    Created,
    LastModified,

    SectionTypeNum
};

enum class SortDirsType : int
{
    First = 0,
    Last = 1,
    NoSpecify = 2,

    SortDirsTypeNum
};

enum class SortOrderType : int
{
    Ascending = Qt::AscendingOrder,
    Descending = Qt::DescendingOrder,
};

enum class SortCaseSensitivity : int
{
    Insensitive = Qt::CaseInsensitive,
    Sensitive = Qt::CaseSensitive,
};

enum class ColorRoleType : int
{
    Unknown = -1,

    Normal,
    Normal_Selected,
    Folder,
    Folder_Selected,
    ReadOnly,
    ReadOnly_Selected,
    Hidden,
    Hidden_Selected,
    System,
    System_Selected,

    Background,
    Selected_Background,

    FolderViewColorRoleTypeNum
};

enum class FileSizeFormatType : int
{
    SI,                     // QLocale::FileSizeSIFormat
    IEC,                    // QLocale::FileSizeIecFormat
    Detail,

    FileSizeFormatTypeNum,
};

enum class PermissionsFormatType : int
{
    Absolute,               // ex.0755
    Symbolic,               // ex.-rwxr-xr-x

    PermissionsFormatTypeNum,
};

enum class DateFormatType : int
{
    Default,                // Qt::TextDate
    ISO,                    // Qt::ISODate
    Original,

    DateFormatTypeNum,
};

// フィルタ
enum class FilterFlag : int
{
    None = 0,

    Dirs   = (1 << 0),      // ディレクトリ
    Files  = (1 << 1),      // ファイル
    Drives = (1 << 2),      // ドライブ
    AllEntrys = Dirs | Files | Drives,

    Hidden = (1 << 16),     // 隠しファイル
    System = (1 << 17),     // システムファイル(Windows)
};
Q_DECLARE_FLAGS(FilterFlags, FilterFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(FilterFlags)

class FolderModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit FolderModel(QObject *parent = Q_NULLPTR);
    ~FolderModel() Q_DECL_OVERRIDE;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    int refresh();

    using QAbstractTableModel::index;
    QModelIndex index(const QString &path) const;

    QModelIndex setRootPath(const QString& path);
    QString rootPath() const;

    /// Filter

    void setFilterFlags(FilterFlags filterFlags);
    FilterFlags filterFlags() const;
    void setNameFilters(const QStringList &nameFilters);
    QStringList nameFilters() const;

    /// Sort

    void setSortSectionType(SectionType sectionType);
    SectionType sortSectionType() const;
    void setSortSectionType2nd(SectionType sectionType2nd);
    SectionType sortSectionType2nd() const;
    void setSortDirsType(SortDirsType dirsType);
    SortDirsType sortDirsType() const;
    void setSortDotFirst(bool dotFirst);
    bool sortDotFirst() const;
    void setSortOrder(SortOrderType order);
    SortOrderType sortOrder() const;
    void setSortCaseSensitivity(SortCaseSensitivity sensitivity);
    SortCaseSensitivity sortCaseSensitivity() const;

    /// Format

    void setFileSizeFormatType(FileSizeFormatType formatType);
    FileSizeFormatType fileSizeFormatType() const;
    void setFileSizeComma(bool comma);
    bool fileSizeComma() const;
    void setDateFormatType(DateFormatType formatType);
    DateFormatType dateFormatType() const;
    void setDateFormatOriginalString(const QString& orgString);
    QString dateFormatOriginalString() const;

    /// File information

    QFileInfo fileInfo(const QModelIndex &index) const;
    bool isDir(const QModelIndex &index) const;
    QString filePath(const QModelIndex &index) const;
    QString fileName(const QModelIndex &index) const;
    qint64 size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QFile::Permissions permissions(const QModelIndex &index) const;
    QDateTime created(const QModelIndex &index) const;
    QDateTime lastModified(const QModelIndex &index) const;
    QIcon fileIcon(const QModelIndex &index) const;

    /// Operation

    QModelIndex mkdir(const QString &name);
    bool rmdir(const QModelIndex &index);
    bool remove(const QModelIndex &index);

    /// Appearance

    void setFont(const QFont& font);
    void initBrushes(const QMap<ColorRoleType, QColor>& colors, bool folderColorTopPrio);

Q_SIGNALS:
    void rootPathChanged(const QString& path);

private:
    QBrush textBrush(const QModelIndex& index) const;
    QBrush backgroundBrush(const QModelIndex& index) const;
    QBrush brush(ColorRoleType colorRole) const;

    bool isSelected(const QModelIndex& index) const;

    void emitRootPathChanged(const QString& path);

    bool lessThan(const QFileInfo& l_info, const QFileInfo& r_info) const;
    bool sectionTypeLessThan(const QFileInfo& l_info, const QFileInfo& r_info,
                             SectionType sectionType, SectionType sectionType2nd, SortCaseSensitivity caseSensitivity) const;

    enum Roles
    {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole = Qt::UserRole + 2,
        FilePermissions = Qt::UserRole + 3
    };

    QItemSelectionModel m_itemSelectionModel;
    QFileIconProvider   m_fileIconProvider;
    QFileSystemWatcher  m_fileSystemWatcher;

    QString m_rootPath;

    QDir m_dir;

    QFileInfoList m_fileInfoList;

    QList<SectionType> m_sectionTypeList;

    FilterFlags m_filterFlags;
    QStringList m_nameFilters;

    SectionType m_sortSectionType;
    SectionType m_sortSectionType2nd;
    SortDirsType m_sortDirsType;
    bool m_sortDotFirst;
    SortOrderType m_sortOrder;
    SortCaseSensitivity m_sortCaseSensitivity;

    FileSizeFormatType m_fileSizeFormatType;
    bool m_fileSizeComma;

    PermissionsFormatType m_permissionsFormatType;

    DateFormatType m_dateFormatType;
    QString m_dateFormatOriginalString;

    QFont m_font;
    QMap<ColorRoleType, QBrush> m_brushes;

    int m_iconSize;

    bool m_folderColorTopPriority;
};

}           // namespace Farman

#endif // DIRMODEL_H
