#include "BulkEditDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>
#include <cmath>

BulkEditDialog::BulkEditDialog(const QVector<TypeItem> &selectedItems, QWidget *parent)
    : QDialog(parent), m_items(selectedItems)
{
    setWindowTitle(QString("Bulk Edit — %1 item(s) selected").arg(m_items.size()));
    setMinimumWidth(480);
    buildUi();
}

// ────────────────────────────────────────────────────────────
// buildUi
// ────────────────────────────────────────────────────────────
void BulkEditDialog::buildUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *infoLabel = new QLabel(
        QString("<b>%1 item(s)</b> selected. Check a field to include it in the bulk edit.")
            .arg(m_items.size()),
        this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // ── Grid for the fields ──
    auto *fieldsGroup  = new QGroupBox("Fields to modify", this);
    auto *grid         = new QGridLayout(fieldsGroup);
    grid->setColumnStretch(3, 1);

    // Header labels
    grid->addWidget(new QLabel("<b>Field</b>",     this), 0, 0);
    grid->addWidget(new QLabel("<b>Mode</b>",      this), 0, 1);
    grid->addWidget(new QLabel("<b>Value</b>",     this), 0, 2);

    // Initial values from the first item (if any)
    const TypeItem &first = m_items.isEmpty() ? TypeItem{} : m_items.first();

    struct FieldDef { QString id; QString label; int value; };
    const QVector<FieldDef> defs = {
        { "nominal",  "Nominal",  first.nominal  },
        { "lifetime", "Lifetime", first.lifetime },
        { "restock",  "Restock",  first.restock  },
        { "min",      "Min",      first.min      },
        { "quantmin", "QuantMin", first.quantmin },
        { "quantmax", "QuantMax", first.quantmax },
        { "cost",     "Cost",     first.cost     },
    };

    for (int i = 0; i < defs.size(); ++i) {
        FieldRow row = createFieldRow(defs[i].id, defs[i].label, defs[i].value,
                                      fieldsGroup, grid, i + 1);
        m_rows.append(row);
    }

    mainLayout->addWidget(fieldsGroup);

    // ── Preview label ──
    m_previewLabel = new QLabel("No fields selected.", this);
    m_previewLabel->setWordWrap(true);
    mainLayout->addWidget(m_previewLabel);

    // ── Buttons ──
    auto *buttonBox = new QDialogButtonBox(this);
    auto *applyBtn  = buttonBox->addButton("Apply",  QDialogButtonBox::AcceptRole);
    auto *cancelBtn = buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);

    connect(applyBtn,  &QPushButton::clicked, this, &BulkEditDialog::onApplyClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);

    updatePreviewLabel();
}

// ────────────────────────────────────────────────────────────
// createFieldRow — creates a field row in the grid
// ────────────────────────────────────────────────────────────
BulkEditDialog::FieldRow BulkEditDialog::createFieldRow(
    const QString &name, const QString &label,
    int currentValue, QWidget * /*parent*/, QGridLayout *grid, int gridRow)
{
    FieldRow row;
    row.name = name;
    row.label = label;

    // Checkbox enable
    row.enableChk = new QCheckBox(label, this);
    grid->addWidget(row.enableChk, gridRow, 0);

    // Mode combo
    row.modeCombo = new QComboBox(this);
    row.modeCombo->addItem("Set",      static_cast<int>(EditMode::Set));
    row.modeCombo->addItem("Add",      static_cast<int>(EditMode::Add));
    row.modeCombo->addItem("Multiply", static_cast<int>(EditMode::Multiply));
    row.modeCombo->setEnabled(false);
    grid->addWidget(row.modeCombo, gridRow, 1);

    // Value spinbox
    row.valueSpinBox = new QDoubleSpinBox(this);
    row.valueSpinBox->setRange(-999999, 999999);
    row.valueSpinBox->setDecimals(2);
    row.valueSpinBox->setValue(currentValue);
    row.valueSpinBox->setEnabled(false);
    grid->addWidget(row.valueSpinBox, gridRow, 2);

    // Enable/disable controls
    connect(row.enableChk, &QCheckBox::toggled, row.modeCombo,    &QComboBox::setEnabled);
    connect(row.enableChk, &QCheckBox::toggled, row.valueSpinBox, &QDoubleSpinBox::setEnabled);
    connect(row.enableChk,    &QCheckBox::toggled,                this, &BulkEditDialog::updatePreviewLabel);
    connect(row.modeCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BulkEditDialog::updatePreviewLabel);
    connect(row.valueSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BulkEditDialog::updatePreviewLabel);

    return row;
}

// ────────────────────────────────────────────────────────────
// updatePreviewLabel — shows a preview of the changes
// ────────────────────────────────────────────────────────────
void BulkEditDialog::updatePreviewLabel()
{
    QStringList parts;

    for (const FieldRow &row : m_rows) {
        if (!row.enableChk->isChecked()) continue;

        const double val = row.valueSpinBox->value();
        const int    mode = row.modeCombo->currentData().toInt();

        QString modeStr;
        switch (static_cast<EditMode>(mode)) {
        case EditMode::Set:      modeStr = QString("= %1").arg(val);  break;
        case EditMode::Add:      modeStr = QString("+ %1").arg(val);  break;
        case EditMode::Multiply: modeStr = QString("× %1").arg(val);  break;
        }
        parts << QString("%1 %2").arg(row.label, modeStr);
    }

    if (parts.isEmpty()) {
        m_previewLabel->setText("No fields selected.");
    } else {
        m_previewLabel->setText(
            QString("Will apply to <b>%1</b> item(s): %2")
                .arg(m_items.size())
                .arg(parts.join(" | ")));
    }
}

// ────────────────────────────────────────────────────────────
// onApplyClicked
// ────────────────────────────────────────────────────────────
void BulkEditDialog::onApplyClicked()
{
    accept();
}

// ────────────────────────────────────────────────────────────
// fieldEdit — returns the edit for a field
// ────────────────────────────────────────────────────────────
BulkEditDialog::FieldEdit BulkEditDialog::fieldEdit(const QString &fieldName) const
{
    for (const FieldRow &row : m_rows) {
        if (row.name == fieldName) {
            FieldEdit fe;
            fe.enabled = row.enableChk->isChecked();
            fe.mode    = static_cast<EditMode>(row.modeCombo->currentData().toInt());
            fe.value   = row.valueSpinBox->value();
            return fe;
        }
    }
    return {};
}

QMap<QString, BulkEditDialog::FieldEdit> BulkEditDialog::allEdits() const
{
    QMap<QString, FieldEdit> result;
    for (const FieldRow &row : m_rows) {
        FieldEdit fe;
        fe.enabled = row.enableChk->isChecked();
        fe.mode    = static_cast<EditMode>(row.modeCombo->currentData().toInt());
        fe.value   = row.valueSpinBox->value();
        result[row.name] = fe;
    }
    return result;
}

// ────────────────────────────────────────────────────────────
// applyEdits — static application of edits to an item
// ────────────────────────────────────────────────────────────
TypeItem BulkEditDialog::applyEdits(const TypeItem &item,
                                     const QMap<QString, FieldEdit> &edits)
{
    TypeItem result = item;

    // Helper lambda for applying a FieldEdit to an int
    auto applyInt = [](int original, const FieldEdit &fe) -> int {
        if (!fe.enabled) return original;
        switch (fe.mode) {
        case EditMode::Set:      return static_cast<int>(fe.value);
        case EditMode::Add:      return original + static_cast<int>(fe.value);
        case EditMode::Multiply: return static_cast<int>(std::round(original * fe.value));
        }
        return original;
    };

    if (edits.contains("nominal"))  result.nominal  = applyInt(item.nominal,  edits["nominal"]);
    if (edits.contains("lifetime")) result.lifetime = applyInt(item.lifetime, edits["lifetime"]);
    if (edits.contains("restock"))  result.restock  = applyInt(item.restock,  edits["restock"]);
    if (edits.contains("min"))      result.min      = applyInt(item.min,      edits["min"]);
    if (edits.contains("quantmin")) result.quantmin = applyInt(item.quantmin, edits["quantmin"]);
    if (edits.contains("quantmax")) result.quantmax = applyInt(item.quantmax, edits["quantmax"]);
    if (edits.contains("cost"))     result.cost     = applyInt(item.cost,     edits["cost"]);

    if (result != item)
        result.modified = true;

    return result;
}
