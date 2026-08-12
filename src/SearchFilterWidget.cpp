#include "SearchFilterWidget.h"
#include "TypesModel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSet>

SearchFilterWidget::SearchFilterWidget(QWidget *parent)
    : QWidget(parent)
{
    buildUi();
}

// ────────────────────────────────────────────────────────────
// buildUi — builds the UI
// ────────────────────────────────────────────────────────────
void SearchFilterWidget::buildUi()
{
    setMinimumWidth(200);
    setMaximumWidth(280);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── Title ──
    auto *titleLabel = new QLabel("<b>Search &amp; Filter</b>", this);
    mainLayout->addWidget(titleLabel);

    // ── Name search ──
    auto *searchGroup = new QGroupBox("Search", this);
    auto *searchLayout = new QVBoxLayout(searchGroup);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Filter by name...");
    m_searchEdit->setClearButtonEnabled(true);
    searchLayout->addWidget(m_searchEdit);
    mainLayout->addWidget(searchGroup);

    // ── Filter dropdowns ──
    auto *filterGroup = new QGroupBox("Filters", this);
    auto *formLayout  = new QFormLayout(filterGroup);
    formLayout->setSpacing(6);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem("(All)");
    formLayout->addRow("Category:", m_categoryCombo);

    m_usageCombo = new QComboBox(this);
    m_usageCombo->addItem("(All)");
    formLayout->addRow("Usage:", m_usageCombo);

    m_valueCombo = new QComboBox(this);
    m_valueCombo->addItem("(All)");
    formLayout->addRow("Value/Tier:", m_valueCombo);

    mainLayout->addWidget(filterGroup);

    // ── Checkbox "Show modified only" ──
    m_modifiedChk = new QCheckBox("Show modified only", this);
    mainLayout->addWidget(m_modifiedChk);

    // ── Reset button ──
    m_resetBtn = new QPushButton("Reset Filters", this);
    mainLayout->addWidget(m_resetBtn);

    // ── Count label ──
    m_countLabel = new QLabel("0 of 0 items", this);
    m_countLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_countLabel);

    mainLayout->addStretch();

    // ── Connections ──
    connect(m_searchEdit,   &QLineEdit::textChanged,  this, &SearchFilterWidget::onNameChanged);
    connect(m_categoryCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SearchFilterWidget::onCategoryChanged);
    connect(m_usageCombo,    QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SearchFilterWidget::onUsageChanged);
    connect(m_valueCombo,    QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &SearchFilterWidget::onValueChanged);
    connect(m_modifiedChk,  &QCheckBox::stateChanged, this, &SearchFilterWidget::onShowModifiedChanged);
    connect(m_resetBtn,     &QPushButton::clicked,    this, &SearchFilterWidget::onResetClicked);
}

// ────────────────────────────────────────────────────────────
// setModels
// ────────────────────────────────────────────────────────────
void SearchFilterWidget::setModels(TypesModel *sourceModel, TypesProxyModel *proxyModel)
{
    m_sourceModel = sourceModel;
    m_proxyModel  = proxyModel;
}

// ────────────────────────────────────────────────────────────
// refreshDropdowns — fills the comboboxes from the items
// ────────────────────────────────────────────────────────────
void SearchFilterWidget::refreshDropdowns()
{
    if (!m_sourceModel) return;

    QSet<QString> categories, usages, values;

    for (const TypeItem &item : m_sourceModel->items()) {
        if (!item.category.isEmpty())
            categories.insert(item.category);
        for (const QString &u : item.usages)
            usages.insert(u);
        for (const QString &v : item.values)
            values.insert(v);
    }

    // Save the current selection
    const QString curCat   = m_categoryCombo->currentText();
    const QString curUsage = m_usageCombo->currentText();
    const QString curVal   = m_valueCombo->currentText();

    // Refill categories
    m_categoryCombo->blockSignals(true);
    m_categoryCombo->clear();
    m_categoryCombo->addItem("(All)");
    QStringList catList = QStringList(categories.begin(), categories.end());
    catList.sort(Qt::CaseInsensitive);
    m_categoryCombo->addItems(catList);
    {
        int idx = m_categoryCombo->findText(curCat);
        m_categoryCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    m_categoryCombo->blockSignals(false);

    // Refill usages
    m_usageCombo->blockSignals(true);
    m_usageCombo->clear();
    m_usageCombo->addItem("(All)");
    QStringList usageList = QStringList(usages.begin(), usages.end());
    usageList.sort(Qt::CaseInsensitive);
    m_usageCombo->addItems(usageList);
    {
        int idx = m_usageCombo->findText(curUsage);
        m_usageCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    m_usageCombo->blockSignals(false);

    // Refill values
    m_valueCombo->blockSignals(true);
    m_valueCombo->clear();
    m_valueCombo->addItem("(All)");
    QStringList valList = QStringList(values.begin(), values.end());
    valList.sort(Qt::CaseInsensitive);
    m_valueCombo->addItems(valList);
    {
        int idx = m_valueCombo->findText(curVal);
        m_valueCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    m_valueCombo->blockSignals(false);
}

void SearchFilterWidget::updateCountLabel(int visible, int total)
{
    m_countLabel->setText(QString("%1 of %2 items").arg(visible).arg(total));
}

void SearchFilterWidget::focusSearch()
{
    m_searchEdit->setFocus();
    m_searchEdit->selectAll();
}

// ────────────────────────────────────────────────────────────
// Slots
// ────────────────────────────────────────────────────────────
void SearchFilterWidget::onNameChanged(const QString &text)
{
    if (m_proxyModel) m_proxyModel->setNameFilter(text);
    emit filtersChanged();
}

void SearchFilterWidget::onCategoryChanged(const QString &text)
{
    if (m_proxyModel) m_proxyModel->setCategoryFilter(text);
    emit filtersChanged();
}

void SearchFilterWidget::onUsageChanged(const QString &text)
{
    if (m_proxyModel) m_proxyModel->setUsageFilter(text);
    emit filtersChanged();
}

void SearchFilterWidget::onValueChanged(const QString &text)
{
    if (m_proxyModel) m_proxyModel->setValueFilter(text);
    emit filtersChanged();
}

void SearchFilterWidget::onShowModifiedChanged(int state)
{
    if (m_proxyModel) m_proxyModel->setShowModifiedOnly(state == Qt::Checked);
    emit filtersChanged();
}

void SearchFilterWidget::onResetClicked()
{
    m_searchEdit->clear();
    m_categoryCombo->setCurrentIndex(0);
    m_usageCombo->setCurrentIndex(0);
    m_valueCombo->setCurrentIndex(0);
    m_modifiedChk->setChecked(false);

    if (m_proxyModel) m_proxyModel->resetFilters();
    emit filtersChanged();
}
