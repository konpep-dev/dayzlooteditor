#pragma once

#include <QWidget>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;
class TypesProxyModel;
class TypesModel;

// ────────────────────────────────────────────────────────────
// SearchFilterWidget — side panel for search / filtering
// ────────────────────────────────────────────────────────────
class SearchFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchFilterWidget(QWidget *parent = nullptr);

    // Connects the widget to the models
    void setModels(TypesModel *sourceModel, TypesProxyModel *proxyModel);

    // Refreshes the category/usage/value dropdowns from the existing items
    void refreshDropdowns();

    // Updates the "X of Y items" label
    void updateCountLabel(int visible, int total);

    // Focuses the search box
    void focusSearch();

signals:
    void filtersChanged();

private slots:
    void onNameChanged(const QString &text);
    void onCategoryChanged(const QString &text);
    void onUsageChanged(const QString &text);
    void onValueChanged(const QString &text);
    void onShowModifiedChanged(int state);
    void onResetClicked();

private:
    TypesModel      *m_sourceModel = nullptr;
    TypesProxyModel *m_proxyModel  = nullptr;

    QLineEdit  *m_searchEdit    = nullptr;
    QComboBox  *m_categoryCombo = nullptr;
    QComboBox  *m_usageCombo    = nullptr;
    QComboBox  *m_valueCombo    = nullptr;
    QCheckBox  *m_modifiedChk   = nullptr;
    QPushButton *m_resetBtn     = nullptr;
    QLabel      *m_countLabel   = nullptr;

    void buildUi();
};
