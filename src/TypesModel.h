#pragma once

#include "TypeItem.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QVector>

// ────────────────────────────────────────────────────────────
// Columns enum — convenient column references
// ────────────────────────────────────────────────────────────
enum Column {
    COL_NAME = 0,
    COL_NOMINAL,
    COL_LIFETIME,
    COL_RESTOCK,
    COL_MIN,
    COL_QUANTMIN,
    COL_QUANTMAX,
    COL_COST,
    COL_CATEGORY,
    COL_USAGES,
    COL_VALUES,
    COL_FLAGS,
    COL_COUNT  // Number of columns
};

// ────────────────────────────────────────────────────────────
// TypesModel — QAbstractTableModel for the TypeItem list
// ────────────────────────────────────────────────────────────
class TypesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TypesModel(QObject *parent = nullptr);

    // ── QAbstractTableModel interface ──
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // ── Data ──
    void setItems(const QVector<TypeItem> &items);
    const QVector<TypeItem> &items() const { return m_items; }
    QVector<TypeItem>       &itemsRef()    { return m_items; }

    TypeItem  item(int row)  const { return m_items.at(row); }
    TypeItem &itemRef(int row)     { return m_items[row]; }

    void setItem(int row, const TypeItem &item);

    // Add / remove
    void appendItem(const TypeItem &item);
    void insertItem(int row, const TypeItem &item);
    void removeItem(int row);
    void removeItems(const QList<int> &rows);

    // Marks all items as saved
    void clearModifiedFlags();

    // Number of modified items
    int modifiedCount() const;

signals:
    void itemModified(int row);

private:
    QVector<TypeItem> m_items;
};

// ────────────────────────────────────────────────────────────
// TypesProxyModel — filtering + sorting
// ────────────────────────────────────────────────────────────
class TypesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TypesProxyModel(QObject *parent = nullptr);

    // Filters
    void setNameFilter(const QString &text);
    void setCategoryFilter(const QString &category);
    void setUsageFilter(const QString &usage);
    void setValueFilter(const QString &value);
    void setShowModifiedOnly(bool only);

    void resetFilters();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_nameFilter;
    QString m_categoryFilter;
    QString m_usageFilter;
    QString m_valueFilter;
    bool    m_showModifiedOnly = false;
};
