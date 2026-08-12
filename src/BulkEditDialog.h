#pragma once

#include "TypeItem.h"
#include <QDialog>
#include <QVector>

class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QLabel;
class QPushButton;

// ────────────────────────────────────────────────────────────
// BulkEditDialog — bulk editing of selected items
// ────────────────────────────────────────────────────────────
class BulkEditDialog : public QDialog
{
    Q_OBJECT

public:
    // Value application mode
    enum class EditMode {
        Set,      // Set a specific value
        Add,      // Add a value
        Multiply  // Multiply by a value
    };

    explicit BulkEditDialog(const QVector<TypeItem> &selectedItems, QWidget *parent = nullptr);

    // Per-field data
    struct FieldEdit {
        bool    enabled   = false;
        EditMode mode     = EditMode::Set;
        double  value     = 0.0;
    };

    // Returns the settings for a field
    FieldEdit fieldEdit(const QString &fieldName) const;

    // Applies the changes to the item and returns the modified copy
    static TypeItem applyEdits(const TypeItem &item,
                               const QMap<QString, FieldEdit> &edits);

    // Returns all edits
    QMap<QString, FieldEdit> allEdits() const;

private slots:
    void onApplyClicked();
    void updatePreviewLabel();

private:
    struct FieldRow {
        QString       name;       // internal field name
        QString       label;      // display label
        QCheckBox    *enableChk;
        QComboBox    *modeCombo;
        QDoubleSpinBox *valueSpinBox;
    };

    QVector<TypeItem>  m_items;
    QVector<FieldRow>  m_rows;
    QLabel            *m_previewLabel = nullptr;

    void buildUi();
    FieldRow createFieldRow(const QString &name, const QString &label,
                            int currentValue, QWidget *parent, class QGridLayout *grid, int gridRow);
};
