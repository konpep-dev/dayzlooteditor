#pragma once

#include "TypeItem.h"
#include "TypesModel.h"

#include <QMainWindow>
#include <QUndoStack>

// Forward declarations
class SearchFilterWidget;
class BackupManager;
class XmlParser;
class QTableView;
class QLabel;
class QSplitter;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QGroupBox;
class QAction;
class QUndoStack;

// ────────────────────────────────────────────────────────────
// MainWindow — main application window
// ────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // Opens a file on startup (command line argument)
    void openFileOnStartup(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // ── File menu ──
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onOpenRecentFile(const QString &path);
    void onExit();

    // ── Edit menu ──
    void onSelectAll();
    void onDeselectAll();
    void onDeleteSelected();

    // ── View menu ──
    void onToggleSearchPanel();
    void onToggleStatusBar();
    void onToggleToolbar();

    // ── Backup menu ──
    void onCreateBackup();
    void onRestoreBackup();
    void onToggleAutoBackup();
    void onOpenBackupFolder();

    // ── Tools menu ──
    void onBulkEditSelected();
    void onAddNewItem();
    void onDuplicateSelected();
    void onSortByName();
    void onMergeXml();

    // ── Table / selection ──
    void onSelectionChanged();
    void onTableDoubleClicked(const QModelIndex &index);
    void onTableContextMenu(const QPoint &pos);
    void onProxyDataChanged();

    // ── Detail panel ──
    void onDetailApply();
    void onDetailRevert();

    // ── Status updates ──
    void updateStatusBar();
    void updateTitle();
    void updateCountLabel();

    // ── Auto-backup timer ──
    void onAutoBackupTimer();

private:
    // ── UI construction ──
    void buildMenus();
    void buildToolbar();
    void buildCentralWidget();
    void buildDetailPanel();
    void buildStatusBar();
    void setupShortcuts();
    void setupAutoBackupTimer();

    // ── Helpers ──
    bool promptSaveIfModified();
    void loadFile(const QString &path);
    void doSave(const QString &path);
    void addRecentFile(const QString &path);
    void updateRecentFilesMenu();

    // Gets the source rows from the table selection
    QList<int> selectedSourceRows() const;

    // Loads the selected item into the detail panel
    void loadItemToDetailPanel(const TypeItem &item);
    TypeItem itemFromDetailPanel() const;

    // ── Models ──
    TypesModel      *m_model      = nullptr;
    TypesProxyModel *m_proxyModel = nullptr;

    // ── Widgets ──
    QSplitter         *m_splitter       = nullptr;
    SearchFilterWidget *m_searchWidget  = nullptr;
    QTableView        *m_tableView      = nullptr;
    QWidget           *m_detailWidget   = nullptr;
    QSplitter         *m_mainSplitter   = nullptr; // horizontal table+detail

    // Detail panel fields
    QLineEdit  *m_dName      = nullptr;
    QSpinBox   *m_dNominal   = nullptr;
    QSpinBox   *m_dLifetime  = nullptr;
    QSpinBox   *m_dRestock   = nullptr;
    QSpinBox   *m_dMin       = nullptr;
    QSpinBox   *m_dQuantMin  = nullptr;
    QSpinBox   *m_dQuantMax  = nullptr;
    QSpinBox   *m_dCost      = nullptr;
    QCheckBox  *m_dCargo     = nullptr;
    QCheckBox  *m_dHoarder   = nullptr;
    QCheckBox  *m_dMap       = nullptr;
    QCheckBox  *m_dPlayer    = nullptr;
    QCheckBox  *m_dCrafted   = nullptr;
    QCheckBox  *m_dDeloot    = nullptr;
    QLineEdit  *m_dCategory  = nullptr;
    QLineEdit  *m_dUsages    = nullptr;
    QLineEdit  *m_dValues    = nullptr;
    QPushButton *m_dApplyBtn  = nullptr;
    QPushButton *m_dRevertBtn = nullptr;
    QGroupBox  *m_detailGroup = nullptr;

    // ── Status bar labels ──
    QLabel *m_statusFile     = nullptr;
    QLabel *m_statusItems    = nullptr;
    QLabel *m_statusBackup   = nullptr;

    // ── Toolbar ──
    QToolBar *m_toolbar = nullptr;

    // ── Menus / actions ──
    QMenu   *m_recentMenu      = nullptr;
    QAction *m_autoBackupAct   = nullptr;
    QAction *m_searchPanelAct  = nullptr;
    QAction *m_statusBarAct    = nullptr;
    QAction *m_toolbarAct      = nullptr;

    // ── Business logic ──
    XmlParser     *m_parser        = nullptr;
    BackupManager *m_backupManager = nullptr;
    QUndoStack    *m_undoStack     = nullptr;
    QTimer        *m_autoBackupTimer = nullptr;

    // ── State ──
    QString     m_currentFilePath;
    QStringList m_recentFiles;
    int         m_detailSourceRow = -1; // currently loaded row in detail panel

    // ── Item counts ──
    int currentTotalCount() const;
    int currentVisibleCount() const;

    // Helper — focus search box (called from toolbar action)
    void focusSearch();
};
