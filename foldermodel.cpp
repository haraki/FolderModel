#include <QDateTime>
#include <QLocale>
#include <QIcon>
#include <QBrush>
#include <QFontMetrics>
#include <QDebug>
#include "misc.h"
#include "foldermodel.h"
#ifdef Q_OS_WIN
#include "win32.h"
#endif
namespace Farman
{

FolderModel::FolderModel(QObject *parent/* = Q_NULLPTR*/)
    : QAbstractTableModel(parent)
    , m_itemSelectionModel(this)
    , m_fileIconProvider()
    , m_fileSystemWatcher(this)
    , m_rootPath("")
    , m_dir()
    , m_filterFlags(FilterFlag::AllEntrys)
    , m_nameFilters({"*"})
    , m_sortSectionType(SectionType::FileName)
    , m_sortSectionType2nd(SectionType::Unknown)
    , m_sortDirsType(SortDirsType::NoSpecify)
    , m_sortDotFirst(true)
    , m_sortOrder(SortOrderType::Ascending)
    , m_sortCaseSensitivity(SortCaseSensitivity::Insensitive)
    , m_fileSizeFormatType(FileSizeFormatType::SI)
    , m_fileSizeComma(false)
    , m_permissionsFormatType(PermissionsFormatType::Symbolic)
    , m_dateFormatType(DateFormatType::Default)
    , m_dateFormatOriginalString("yyyy-MM-dd HH:mm:ss")
    , m_font()
    , m_brushes()
    , m_iconSize(16)
    , m_folderColorTopPriority(false)
{
    m_sectionTypeList =
    {
        SectionType::FileName,
        SectionType::FileType,
        SectionType::FileSize,
//        SectionType::Owner,
//        SectionType::Group,
//        SectionType::Permissions,
//        SectionType::Created,
        SectionType::LastModified,
    };

    m_dir.setFilter(QDir::AllEntries | QDir::AccessMask | QDir::NoDot);
    m_dir.setNameFilters({"..", "*"});
}

FolderModel::~FolderModel()
{
}

QVariant FolderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant retData = QVariant();

    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        SectionType sectionType = m_sectionTypeList[section];

        switch(sectionType)
        {
        case SectionType::FileName:
            retData = tr("Name");
            break;
        case SectionType::FileType:
            retData = tr("FileType");
            break;
        case SectionType::FileSize:
            retData = tr("Size");
            break;
        case SectionType::Owner:
            retData = tr("Owner");
            break;
        case SectionType::Group:
            retData = tr("Group");
            break;
        case SectionType::Permissions:
            retData = tr("Permissions");
            break;
        case SectionType::Created:
            retData = tr("Created");
            break;
        case SectionType::LastModified:
            retData = tr("Last modified");
            break;
        default:
            break;
        }
    }

    return retData;
}

int FolderModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_fileInfoList.count();
}

int FolderModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_sectionTypeList.count();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.column() >= m_sectionTypeList.count())
    {
        return QVariant();
    }

    QVariant ret = QVariant();

    SectionType sectionType = m_sectionTypeList[index.column()];

    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
        QFileInfo fi = fileInfo(index);

        switch(sectionType)
        {
        case SectionType::FileName:
            if(!fi.isDir() && !fi.completeBaseName().isEmpty())
            {
                ret = fi.completeBaseName();
            }
            else
            {
                ret = fi.fileName();
            }
            break;

        case SectionType::FileType:
            if(!fi.isDir() && !fi.completeBaseName().isEmpty())
            {
                ret = fi.suffix();
            }
            break;

        case SectionType::FileSize:
            if(fi.isDir())
            {
                ret = QString("<Folder>");
            }
            else
            {
                if(m_fileSizeFormatType == FileSizeFormatType::Detail)
                {
                    ret = (m_fileSizeComma) ? QLocale(QLocale::English).toString(fi.size()) : QString::number(fi.size());
                }
                else
                {
                    ret = QLocale().formattedDataSize(fi.size(), 2,
                                                      (m_fileSizeFormatType == FileSizeFormatType::IEC) ? QLocale::DataSizeIecFormat :
                                                                                                          QLocale::DataSizeSIFormat);
                }
            }
            break;

        case SectionType::Owner:
            ret = fi.owner();

            break;

        case SectionType::Group:
            ret = fi.group();

            break;

        case SectionType::Permissions:
            if(m_permissionsFormatType == PermissionsFormatType::Absolute)
            {
                ret = QString::number(static_cast<int>(fi.permissions()) & 0x0FFF, 16);
            }
            else
            {
                QFile::Permissions perms = fi.permissions();

                ret =  QString("%1%2%3%4%5%6%7%8%9%10")
                        .arg(fi.isDir() ? "d" : "-")
                        .arg(perms & QFile::ReadUser   ? "r" : "-")
                        .arg(perms & QFile::WriteUser  ? "w" : "-")
                        .arg(perms & QFile::ExeUser    ? "x" : "-")
                        .arg(perms & QFile::ReadGroup  ? "r" : "-")
                        .arg(perms & QFile::WriteGroup ? "w" : "-")
                        .arg(perms & QFile::ExeGroup   ? "x" : "-")
                        .arg(perms & QFile::ReadOther  ? "r" : "-")
                        .arg(perms & QFile::WriteOther ? "w" : "-")
                        .arg(perms & QFile::ExeOther   ? "x" : "-");
            }

            break;

        case SectionType::Created:
        case SectionType::LastModified:
        {
            QDateTime time = (sectionType == SectionType::Created) ? fi.birthTime() : fi.lastModified();

            switch(m_dateFormatType)
            {
            case DateFormatType::ISO:
                ret = time.toString(Qt::ISODate);
                break;
            case DateFormatType::Original:
            {
                ret = time.toString(m_dateFormatOriginalString);
                break;
            }
            case DateFormatType::Default:
            default:
                ret = time.toString(Qt::TextDate);
                break;
            }
            break;
        }
        default:
            break;
        }

        break;
    }
    case Qt::FontRole:
        ret = m_font;

        break;

    case Qt::TextAlignmentRole:
        if(sectionType == SectionType::FileSize ||
           sectionType == SectionType::Created ||
           sectionType == SectionType::LastModified)
        {
            ret = Qt::AlignRight + Qt::AlignVCenter;
        }
        else
        {
            ret = Qt::AlignLeft + Qt::AlignVCenter;
        }

        break;

    case Qt::TextColorRole:
        ret = textBrush(index);

        break;

    case Qt::BackgroundRole:
        ret = backgroundBrush(index);

        break;

    case FileIconRole:
        if(sectionType == SectionType::FileName)
        {
            ret = fileIcon(index).pixmap(m_iconSize, m_iconSize);
        }

        break;

    case FilePathRole:
        if(sectionType == SectionType::FileName)
        {
            ret = filePath(index);
        }

        break;

    case FileNameRole:
        if(sectionType == SectionType::FileName)
        {
            ret = fileName(index);
        }

        break;

    }

//    qDebug() << "data(" << modelIndex << "," << static_cast<Qt::ItemDataRole>(role) << ") : ret = " << ret;

    return ret;
}

QModelIndex FolderModel::index(int row, int column, const QModelIndex &parent/* = QModelIndex()*/) const
{
    row = Clamp(row, 0, m_fileInfoList.count() - 1);
    column = Clamp(column, 0, m_sectionTypeList.size() - 1);

    QModelIndex ret = QAbstractTableModel::index(row, column, parent);

    if(ret.row() < 0)
    {
        qDebug() << "=================== FolderModel::index() row : " << row << ", column : " << column;
    }

    return ret;
}

QModelIndex FolderModel::index(const QString &path) const
{
    for(int row = 0;row < m_fileInfoList.count();row++)
    {
        if(m_fileInfoList[row].filePath() == path)
        {
            return index(row, 0);
        }
    }

    return QModelIndex();
}

int FolderModel::setRootPath(const QString& path)
{
    if(!QFileInfo::exists(path))
    {
        return -1;
    }

    QString oldPath = m_rootPath;
    m_rootPath = path;

    m_dir.setPath(path);

    int ret = refresh();
    if(ret < 0)
    {
        m_rootPath = oldPath;

        return -1;
    }

    emitRootPathChanged(path);

    return 0;
}

QString FolderModel::rootPath() const
{
    return m_rootPath;
}

int FolderModel::fileNum()
{
    return getFileDirNum(FilterFlag::Files);
}

int FolderModel::dirNum()
{
    return getFileDirNum(FilterFlag::Dirs);
}

int FolderModel::fileDirNum()
{
    return getFileDirNum(FilterFlag::Files | FilterFlag::Dirs);
}

int FolderModel::getFileDirNum(FilterFlags filterFlags)
{
    QFileInfoList fileInfoList = m_dir.entryInfoList();
    if(fileInfoList.isEmpty())
    {
        return 0;
    }

    filterFlags &= m_filterFlags;       // Files / Dirs フラグは、引数(filterFlags)と m_filterFlags の両方で有効でなければならない

    int count = 0;
    foreach(const QFileInfo& fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "..")
        {
            continue;
        }
        else
        {
            if(fileInfo.isDir())
            {
                if(!(filterFlags & FilterFlag::Dirs))
                {
                    continue;
                }
            }
            else if(fileInfo.isFile())
            {
                if(!(filterFlags & FilterFlag::Files))
                {
                    continue;
                }
            }

            if(fileInfo.isHidden() && !(m_filterFlags & FilterFlag::Hidden))
            {
                continue;
            }
#ifdef Q_OS_WIN
            if(Win32::isSystemFile(fileInfo.absoluteFilePath()) && !(m_filterFlags & FilterFlag::System))
            {
                continue;
            }
#endif
        }
        count++;
    }

    return count;
}

int FolderModel::refresh()
{
    QFileInfoList fileInfoList = m_dir.entryInfoList();
    if(fileInfoList.isEmpty())
    {
        qDebug() << "Entry list is Empty.";

        return -1;
    }

    beginResetModel();

    m_fileInfoList.clear();

    foreach(const QFileInfo& fileInfo, fileInfoList)
    {
        if(fileInfo.fileName() == "..")
        {
            if(m_dir.isRoot())
            {
                continue;
            }
        }
        else
        {
            if(fileInfo.isDir())
            {
                if(!(m_filterFlags & FilterFlag::Dirs))
                {
                    continue;
                }
            }
            else if(fileInfo.isFile())
            {
                if(!(m_filterFlags & FilterFlag::Files))
                {
                    continue;
                }
            }

            if(fileInfo.isHidden() && !(m_filterFlags & FilterFlag::Hidden))
            {
                continue;
            }
#ifdef Q_OS_WIN
            if(Win32::isSystemFile(fileInfo.absoluteFilePath()) && !(m_filterFlags & FilterFlag::System))
            {
                continue;
            }
#endif
        }
        m_fileInfoList.push_back(fileInfo);
    }

    std::sort(m_fileInfoList.begin(), m_fileInfoList.end(),
          [this](const QFileInfo& l, const QFileInfo& r){ return this->lessThan(l, r); });

    endResetModel();

    return 0;
}

bool FolderModel::lessThan(const QFileInfo& l_info, const QFileInfo& r_info) const
{
//    qDebug() << "FolderModel::lessThan() : source_left : " << l_info.filePath() << ", source_right : " << r_info.filePath();

    if(m_sortDotFirst)
    {
        if(l_info.fileName() == ".")
        {
            return true;
        }
        else if(r_info.fileName() == ".")
        {
            return false;
        }
        else if(l_info.fileName() == ".." && r_info.fileName() != ".")
        {
            return true;
        }
        else if(r_info.fileName() == ".." && l_info.fileName() != ".")
        {
            return false;
        }
    }

    if(m_sortDirsType == SortDirsType::First)
    {
        if(l_info.isDir() && !r_info.isDir())
        {
            return true;
        }
        else if(!l_info.isDir() && r_info.isDir())
        {
            return false;
        }
    }
    else if(m_sortDirsType == SortDirsType::Last)
    {
        if(l_info.isDir() && !r_info.isDir())
        {
            return false;
        }
        else if(!l_info.isDir() && r_info.isDir())
        {
            return true;
        }
    }

    bool ascOrder = (m_sortOrder == SortOrderType::Ascending);

    return sectionTypeLessThan((ascOrder) ? l_info : r_info,
                               (ascOrder) ? r_info : l_info,
                               m_sortSectionType, m_sortSectionType2nd, m_sortCaseSensitivity);
}

bool FolderModel::sectionTypeLessThan(const QFileInfo& l_info, const QFileInfo& r_info,
                                   SectionType sectionType, SectionType sectionType2nd, SortCaseSensitivity caseSensitivity) const
{
    if(sectionType == SectionType::FileSize)
    {
        if(!l_info.isDir() && !r_info.isDir())
        {
            if(sectionType2nd != SectionType::Unknown && l_info.size() == r_info.size())
            {
                return sectionTypeLessThan(l_info, r_info, sectionType2nd, SectionType::Unknown, caseSensitivity);
            }
            else
            {
                return l_info.size() < r_info.size();
            }
        }
        else
        {
            if(sectionType2nd != SectionType::Unknown)
            {
                return sectionTypeLessThan(l_info, r_info, sectionType2nd, SectionType::Unknown, caseSensitivity);
            }
        }
    }
    else if(sectionType == SectionType::FileType)
    {
        QString l_type = (!l_info.isDir() && !l_info.completeBaseName().isEmpty()) ? l_info.suffix() : "";
        QString r_type = (!r_info.isDir() && !r_info.completeBaseName().isEmpty()) ? r_info.suffix() : "";

        if(l_type.isEmpty() && r_type.isEmpty())
        {
            l_type = l_info.fileName();
            r_type = r_info.fileName();
        }

        if(caseSensitivity == SortCaseSensitivity::Insensitive)
        {
            l_type = l_type.toLower();
            r_type = r_type.toLower();
        }

        if(sectionType2nd != SectionType::Unknown && l_type == r_type)
        {
            return sectionTypeLessThan(l_info, r_info, sectionType2nd, SectionType::Unknown, caseSensitivity);
        }
        else
        {
            return l_type < r_type;
        }
    }
    else if(sectionType == SectionType::LastModified)
    {
        if(sectionType2nd != SectionType::Unknown && l_info.lastModified() == r_info.lastModified())
        {
            return sectionTypeLessThan(l_info, r_info, sectionType2nd, SectionType::Unknown, caseSensitivity);
        }
        else
        {
            return l_info.lastModified() < r_info.lastModified();
        }
    }
    else
    {
        QString l_name = (!l_info.isDir() && !l_info.completeBaseName().isEmpty()) ? l_info.completeBaseName() : l_info.fileName();
        QString r_name = (!r_info.isDir() && !r_info.completeBaseName().isEmpty()) ? r_info.completeBaseName() : r_info.fileName();

        if(caseSensitivity == SortCaseSensitivity::Insensitive)
        {
            l_name = l_name.toLower();
            r_name = r_name.toLower();
        }

        if(sectionType2nd != SectionType::Unknown && l_name == r_name)
        {
            return sectionTypeLessThan(l_info, r_info, sectionType2nd, SectionType::Unknown, caseSensitivity);
        }
        else
        {
            return l_name < r_name;
        }
    }

    return false;
}

/// Filter

void FolderModel::setFilterFlags(FilterFlags filterFlags)
{
    m_filterFlags = filterFlags;
}

FilterFlags FolderModel::filterFlags() const
{
    return m_filterFlags;
}

void FolderModel::setNameFilters(const QStringList &nameFilters)
{
    m_nameFilters.clear();

    foreach(const QString& nf, nameFilters)
    {
        if(nf == "." || nf == "..")
        {
            continue;
        }
        m_nameFilters.push_back(nf);
    }

    QStringList _nameFilters = {".."};
    _nameFilters += m_nameFilters;

    m_dir.setNameFilters(_nameFilters);
}

QStringList FolderModel::nameFilters() const
{
    return m_nameFilters;
}

/// Sort

void FolderModel::setSortSectionType(SectionType sectionType)
{
    m_sortSectionType = sectionType;
}

SectionType FolderModel::sortSectionType() const
{
    return m_sortSectionType;
}

void FolderModel::setSortSectionType2nd(SectionType sectionType2nd)
{
    m_sortSectionType2nd = sectionType2nd;
}

SectionType FolderModel::sortSectionType2nd() const
{
    return m_sortSectionType2nd;
}

void FolderModel::setSortDirsType(SortDirsType dirsType)
{
    m_sortDirsType = dirsType;
}

SortDirsType FolderModel::sortDirsType() const
{
    return m_sortDirsType;
}

void FolderModel::setSortDotFirst(bool dotFirst)
{
    m_sortDotFirst = dotFirst;
}

bool FolderModel::sortDotFirst() const
{
    return m_sortDotFirst;
}

void FolderModel::setSortOrder(SortOrderType order)
{
    m_sortOrder = order;
}

SortOrderType FolderModel::sortOrder() const
{
    return m_sortOrder;
}

void FolderModel::setSortCaseSensitivity(SortCaseSensitivity sensitivity)
{
    m_sortCaseSensitivity = sensitivity;
}

SortCaseSensitivity FolderModel::sortCaseSensitivity() const
{
    return m_sortCaseSensitivity;
}

/// Format

void FolderModel::setFileSizeFormatType(FileSizeFormatType formatType)
{
    m_fileSizeFormatType = formatType;
}

FileSizeFormatType FolderModel::fileSizeFormatType() const
{
    return m_fileSizeFormatType;
}

void FolderModel::setFileSizeComma(bool comma)
{
    m_fileSizeComma = comma;
}

bool FolderModel::fileSizeComma() const
{
    return m_fileSizeComma;
}

void FolderModel::setDateFormatType(DateFormatType formatType)
{
    m_dateFormatType = formatType;
}

DateFormatType FolderModel::dateFormatType() const
{
    return m_dateFormatType;
}

void FolderModel::setDateFormatOriginalString(const QString& orgString)
{
    m_dateFormatOriginalString = orgString;
}

QString FolderModel::dateFormatOriginalString() const
{
    return m_dateFormatOriginalString;
}

/// File information

QFileInfo FolderModel::fileInfo(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()];
    }

    return QFileInfo();
}

bool FolderModel::isDir(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].isDir();
    }

    return false;
}

QString FolderModel::filePath(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].filePath();
    }

    return "";
}

QString FolderModel::fileName(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].fileName();
    }

    return "";
}

QIcon FolderModel::fileIcon(const QModelIndex &index) const
{
    return m_fileIconProvider.icon(fileInfo(index));
}

QFile::Permissions FolderModel::permissions(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].permissions();
    }

    return QFile::Permissions();
}

qint64 FolderModel::size(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].size();
    }

    return -1;
}

QString FolderModel::type(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].suffix();
    }

    return "";
}

QDateTime FolderModel::created(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].birthTime();
    }

    return QDateTime();
}

QDateTime FolderModel::lastModified(const QModelIndex &index) const
{
    if(index.row() < m_fileInfoList.count())
    {
        return m_fileInfoList[index.row()].lastModified();
    }

    return QDateTime();
}

/// Appearance

void FolderModel::setFont(const QFont& font)
{
    m_font = font;

    m_iconSize = QFontMetrics(font).height();
}

void FolderModel::initBrushes(const QMap<ColorRoleType, QColor>& colors, bool folderColorTopPrio)
{
    m_brushes.clear();

    for(auto colorRole : colors.keys())
    {
        m_brushes[colorRole] = QBrush(colors[colorRole]);
    }

    m_folderColorTopPriority = folderColorTopPrio;
}

QBrush FolderModel::textBrush(const QModelIndex& index) const
{
    QBrush ret;

    QFileInfo fi = fileInfo(index);
    bool selected = isSelected(index);

    if(m_folderColorTopPriority && fi.isDir())
    {
        if(selected)
        {
            ret = brush(ColorRoleType::Folder_Selected);
        }
        else
        {
            ret = brush(ColorRoleType::Folder);
        }
    }
#ifdef Q_OS_WIN
    else if(fi.fileName() != ".." && Win32::isSystemFile(fi.absoluteFilePath()))
    {
        if(selected)
        {
            ret = brush(FolderViewColorRoleType::System_Selected);
        }
        else
        {
            ret = brush(FolderViewColorRoleType::System);
        }
    }
#endif
    else if((fi.fileName() != "..") && (fi.isHidden()))
    {
        if(selected)
        {
            ret = brush(ColorRoleType::Hidden_Selected);
        }
        else
        {
            ret = brush(ColorRoleType::Hidden);
        }
    }
    else if((fi.fileName() != "..") && (!fi.isWritable()))
    {
        if(selected)
        {
            ret = brush(ColorRoleType::ReadOnly_Selected);
        }
        else
        {
            ret = brush(ColorRoleType::ReadOnly);
        }
    }
    else if(!m_folderColorTopPriority && fi.isDir())
    {
        if(selected)
        {
            ret = brush(ColorRoleType::Folder_Selected);
        }
        else
        {
            ret = brush(ColorRoleType::Folder);
        }
    }
    else
    {
        if(selected)
        {
            ret = brush(ColorRoleType::Normal_Selected);
        }
        else
        {
            ret = brush(ColorRoleType::Normal);
        }
    }

    return ret;
}

QBrush FolderModel::backgroundBrush(const QModelIndex& index) const
{
    QBrush ret;

    if(isSelected(index))
    {
        ret = brush(ColorRoleType::Selected_Background);
    }
    else
    {
        ret = brush(ColorRoleType::Background);
    }

    return ret;
}

QBrush FolderModel::brush(ColorRoleType colorRole) const
{
    QBrush ret;

    QMap<ColorRoleType, QBrush>::const_iterator itr = m_brushes.find(colorRole);
    if(itr != m_brushes.end())
    {
        ret = *itr;
    }

    return ret;
}

/// Select

QItemSelectionModel* FolderModel::selectionModel()
{
    return &m_itemSelectionModel;
}

void FolderModel::setSelect(int row, QItemSelectionModel::SelectionFlags selectionFlags)
{
    QItemSelection selection(index(row, 0), index(row, columnCount() - 1));
    m_itemSelectionModel.select(selection, selectionFlags);
}

void FolderModel::setSelectAll()
{
    for(int row = 0;row < rowCount();row++)
    {
        if(fileName(index(row, 0)) != "..")
        {
            setSelect(row, QItemSelectionModel::Select);
        }
    }
}

QModelIndexList FolderModel::selectedIndexList() const
{
    return m_itemSelectionModel.selectedRows();
}

void FolderModel::clearSelected()
{
    m_itemSelectionModel.clear();
}

bool FolderModel::isSelected(const QModelIndex& index) const
{
    return m_itemSelectionModel.isSelected(index);
}

/// Signal

void FolderModel::emitRootPathChanged(const QString& path)
{
    emit rootPathChanged(path);
}

}           // namespace Farman
