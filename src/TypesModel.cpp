#include "TypesModel.h"

#include <QColor>
#include <QFont>
#include <QBrush>

// ════════════════════════════════════════════════════════════
// TypesModel
// ════════════════════════════════════════════════════════════

TypesModel::TypesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TypesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

int TypesModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : COL_COUNT;
}

// ────────────────────────────────────────────────────────────
// data — returns cell data
// ────────────────────────────────────────────────────────────
QVariant TypesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const TypeItem &item = m_items.at(index.row());
    const int col = index.column();

    // ── Background role: modified rows ──
    if (role == Qt::BackgroundRole && item.modified) {
        return QBrush(QColor(0x2a, 0x2a, 0x1e)); // Light yellow tint
    }

    if (role == Qt::ForegroundRole && item.modified) {
        return QBrush(QColor(0xff, 0xe0, 0x80)); // Light yellow for modified rows
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (col) {
        case COL_NAME:     return item.name;
        case COL_NOMINAL:  return item.nominal;
        case COL_LIFETIME: return item.lifetime;
        case COL_RESTOCK:  return item.restock;
        case COL_MIN:      return item.min;
        case COL_QUANTMIN: return item.quantmin;
        case COL_QUANTMAX: return item.quantmax;
        case COL_COST:     return item.cost;
        case COL_CATEGORY: return item.category;
        case COL_USAGES:   return item.usages.join(", ");
        case COL_VALUES:   return item.values.join(", ");
        case COL_FLAGS:    return item.flagsSummary();
        default: break;
        }
    }

    // ── ToolTip: full flags info ──
    if (role == Qt::ToolTipRole && col == COL_FLAGS) {
        return QString("map=%1 cargo=%2 hoarder=%3 player=%4 crafted=%5 deloot=%6")
            .arg(item.count_in_map)
            .arg(item.count_in_cargo)
            .arg(item.count_in_hoarder)
            .arg(item.count_in_player)
            .arg(item.crafted)
            .arg(item.deloot);
    }

    return {};
}

// ────────────────────────────────────────────────────────────
// setData — inline cell editing
// ────────────────────────────────────────────────────────────
bool TypesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    if (index.row() >= m_items.size())
        return false;

    TypeItem &item = m_items[index.row()];
    const int col  = index.column();
    bool ok = false;

    switch (col) {
    case COL_NAME:
        item.name = value.toString();
        ok = true;
        break;
    case COL_NOMINAL:
        item.nominal = value.toInt(&ok);
        break;
    case COL_LIFETIME:
        item.lifetime = value.toInt(&ok);
        break;
    case COL_RESTOCK:
        item.restock = value.toInt(&ok);
        break;
    case COL_MIN:
        item.min = value.toInt(&ok);
        break;
    case COL_QUANTMIN:
        item.quantmin = value.toInt(&ok);
        break;
    case COL_QUANTMAX:
        item.quantmax = value.toInt(&ok);
        break;
    case COL_COST:
        item.cost = value.toInt(&ok);
        break;
    case COL_CATEGORY:
        item.category = value.toString();
        ok = true;
        break;
    case COL_USAGES: {
        const QString s = value.toString().trimmed();
        item.usages = s.isEmpty() ? QStringList{}
                                  : s.split(',', Qt::SkipEmptyParts);
        for (auto &u : item.usages) u = u.trimmed();
        ok = true;
        break;
    }
    case COL_VALUES: {
        const QString s = value.toString().trimmed();
        item.values = s.isEmpty() ? QStringList{}
                                  : s.split(',', Qt::SkipEmptyParts);
        for (auto &v : item.values) v = v.trimmed();
        ok = true;
        break;
    }
    default:
        break;
    }

    if (ok) {
        item.modified = true;
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole});
        emit itemModified(index.row());
    }

    return ok;
}

// ────────────────────────────────────────────────────────────
// headerData
// ────────────────────────────────────────────────────────────
QVariant TypesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    switch (section) {
    case COL_NAME:     return "Name";
    case COL_NOMINAL:  return "Nominal";
    case COL_LIFETIME: return "Lifetime";
    case COL_RESTOCK:  return "Restock";
    case COL_MIN:      return "Min";
    case COL_QUANTMIN: return "QuantMin";
    case COL_QUANTMAX: return "QuantMax";
    case COL_COST:     return "Cost";
    case COL_CATEGORY: return "Category";
    case COL_USAGES:   return "Usages";
    case COL_VALUES:   return "Values";
    case COL_FLAGS:    return "Flags";
    default: return {};
    }
}

// ────────────────────────────────────────────────────────────
// flags — editable cells
// ────────────────────────────────────────────────────────────
Qt::ItemFlags TypesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // The Flags column is not edited inline (handled by the detail panel)
    if (index.column() != COL_FLAGS)
        f |= Qt::ItemIsEditable;

    return f;
}

// ────────────────────────────────────────────────────────────
// setItems — loads new items
// ────────────────────────────────────────────────────────────
void TypesModel::setItems(const QVector<TypeItem> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void TypesModel::setItem(int row, const TypeItem &item)
{
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row] = item;
    emit dataChanged(index(row, 0), index(row, COL_COUNT - 1),
                     {Qt::DisplayRole, Qt::EditRole, Qt::BackgroundRole});
}

void TypesModel::appendItem(const TypeItem &item)
{
    const int row = m_items.size();
    beginInsertRows({}, row, row);
    m_items.append(item);
    endInsertRows();
}

void TypesModel::insertItem(int row, const TypeItem &item)
{
    row = qBound(0, row, m_items.size());
    beginInsertRows({}, row, row);
    m_items.insert(row, item);
    endInsertRows();
}

void TypesModel::removeItem(int row)
{
    if (row < 0 || row >= m_items.size())
        return;
    beginRemoveRows({}, row, row);
    m_items.removeAt(row);
    endRemoveRows();
}

void TypesModel::removeItems(const QList<int> &rows)
{
    // Delete from larger index to smaller
    QList<int> sorted = rows;
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    for (int row : sorted) {
        removeItem(row);
    }
}

void TypesModel::clearModifiedFlags()
{
    for (TypeItem &item : m_items)
        item.modified = false;

    if (!m_items.isEmpty()) {
        emit dataChanged(index(0, 0),
                         index(m_items.size() - 1, COL_COUNT - 1),
                         {Qt::BackgroundRole, Qt::ForegroundRole});
    }
}

int TypesModel::modifiedCount() const
{
    int n = 0;
    for (const TypeItem &item : m_items)
        if (item.modified) ++n;
    return n;
}

// ════════════════════════════════════════════════════════════
// TypesProxyModel
// ════════════════════════════════════════════════════════════

TypesProxyModel::TypesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

void TypesProxyModel::setNameFilter(const QString &text)
{
    m_nameFilter = text;
    invalidateFilter();
}

void TypesProxyModel::setCategoryFilter(const QString &category)
{
    m_categoryFilter = category;
    invalidateFilter();
}

void TypesProxyModel::setUsageFilter(const QString &usage)
{
    m_usageFilter = usage;
    invalidateFilter();
}

void TypesProxyModel::setValueFilter(const QString &value)
{
    m_valueFilter = value;
    invalidateFilter();
}

void TypesProxyModel::setShowModifiedOnly(bool only)
{
    m_showModifiedOnly = only;
    invalidateFilter();
}

void TypesProxyModel::resetFilters()
{
    m_nameFilter.clear();
    m_categoryFilter.clear();
    m_usageFilter.clear();
    m_valueFilter.clear();
    m_showModifiedOnly = false;
    invalidateFilter();
}

bool TypesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*parent*/) const
{
    auto *src = qobject_cast<TypesModel *>(sourceModel());
    if (!src || sourceRow >= src->items().size())
        return false;

    const TypeItem &item = src->items().at(sourceRow);

    // Name filter
    if (!m_nameFilter.isEmpty() &&
        !item.name.contains(m_nameFilter, Qt::CaseInsensitive))
        return false;

    // Category filter
    if (!m_categoryFilter.isEmpty() && m_categoryFilter != "(All)" &&
        item.category != m_categoryFilter)
        return false;

    // Usage filter
    if (!m_usageFilter.isEmpty() && m_usageFilter != "(All)") {
        bool found = false;
        for (const QString &u : item.usages) {
            if (u.compare(m_usageFilter, Qt::CaseInsensitive) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    // Value filter
    if (!m_valueFilter.isEmpty() && m_valueFilter != "(All)") {
        bool found = false;
        for (const QString &v : item.values) {
            if (v.compare(m_valueFilter, Qt::CaseInsensitive) == 0) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    // Show only modified items
    if (m_showModifiedOnly && !item.modified)
        return false;

    return true;
}
