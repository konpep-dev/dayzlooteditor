#include "MainWindow.h"
#include "SearchFilterWidget.h"
#include "BulkEditDialog.h"
#include "BackupManager.h"
#include "XmlParser.h"
#include "DarkTheme.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTableView>
#include <QTimer>
#include <QToolBar>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QFileInfo>
#include <QDir>
#include <QFile>

// ════════════════════════════════════════════════════════════
//  Undo commands
// ════════════════════════════════════════════════════════════

// Command for changing one item
class EditItemCommand : public QUndoCommand
{
public:
    EditItemCommand(TypesModel *model, int row,
                    const TypeItem &oldItem, const TypeItem &newItem,
                    const QString &description = "Edit Item")
        : QUndoCommand(description), m_model(model), m_row(row),
          m_old(oldItem), m_new(newItem)
    {}

    void undo() override { m_model->setItem(m_row, m_old); }
    void redo() override { m_model->setItem(m_row, m_new); }

private:
    TypesModel *m_model;
    int         m_row;
    TypeItem    m_old, m_new;
};

// Command for adding an item
class AddItemCommand : public QUndoCommand
{
public:
    AddItemCommand(TypesModel *model, int row, const TypeItem &item)
        : QUndoCommand("Add Item"), m_model(model), m_row(row), m_item(item)
    {}

    void undo() override { m_model->removeItem(m_row); }
    void redo() override { m_model->insertItem(m_row, m_item); }

private:
    TypesModel *m_model;
    int         m_row;
    TypeItem    m_item;
};

// Command for deleting item(s)
class RemoveItemsCommand : public QUndoCommand
{
public:
    RemoveItemsCommand(TypesModel *model, const QList<int> &rows)
        : QUndoCommand("Delete Item(s)"), m_model(model)
    {
        QList<int> sorted = rows;
        std::sort(sorted.begin(), sorted.end());
        for (int r : sorted)
            m_items.append({r, model->item(r)});
    }

    void undo() override {
        for (auto it = m_items.begin(); it != m_items.end(); ++it)
            m_model->insertItem(it->first, it->second);
    }

    void redo() override {
        QList<int> rows;
        for (auto &p : m_items) rows << p.first;
        m_model->removeItems(rows);
    }

private:
    TypesModel *m_model;
    QList<QPair<int, TypeItem>> m_items;
};

// Command for bulk edit
class BulkEditCommand : public QUndoCommand
{
public:
    BulkEditCommand(TypesModel *model, const QList<int> &rows,
                    const QVector<TypeItem> &newItems)
        : QUndoCommand("Bulk Edit"), m_model(model)
    {
        for (int i = 0; i < rows.size(); ++i) {
            m_data.append({rows[i], model->item(rows[i]), newItems[i]});
        }
    }

    void undo() override {
        for (auto &d : m_data) m_model->setItem(std::get<0>(d), std::get<1>(d));
    }
    void redo() override {
        for (auto &d : m_data) m_model->setItem(std::get<0>(d), std::get<2>(d));
    }

private:
    TypesModel *m_model;
    QList<std::tuple<int, TypeItem, TypeItem>> m_data;
};

// ════════════════════════════════════════════════════════════
//  MainWindow — Constructor
// ════════════════════════════════════════════════════════════
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("DayZ Types Editor");
    setMinimumSize(1100, 700);
    resize(1400, 850);

    // ── Business logic objects ──
    m_parser        = new XmlParser();
    m_backupManager = new BackupManager(this);
    m_undoStack     = new QUndoStack(this);

    // ── Model ──
    m_model      = new TypesModel(this);
    m_proxyModel = new TypesProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    // ── UI ──
    buildMenus();
    buildToolbar();
    buildCentralWidget();
    buildStatusBar();
    setupShortcuts();
    setupAutoBackupTimer();

    // ── Settings restore ──
    QSettings settings;
    m_recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFilesMenu();
    bool autoBackup = settings.value("autoBackup", true).toBool();
    m_backupManager->setAutoBackupEnabled(autoBackup);
    m_autoBackupAct->setChecked(autoBackup);

    // Restore geometry
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    updateTitle();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete m_parser;
}

// ────────────────────────────────────────────────────────────
// buildMenus
// ────────────────────────────────────────────────────────────
void MainWindow::buildMenus()
{
    QMenuBar *mb = menuBar();

    // ══ File ══
    QMenu *fileMenu = mb->addMenu("&File");

    fileMenu->addAction("&New",     this, &MainWindow::onNewFile,    QKeySequence::New);
    fileMenu->addAction("&Open...", this, &MainWindow::onOpenFile,   QKeySequence::Open);
    fileMenu->addAction("&Save",    this, &MainWindow::onSaveFile,   QKeySequence::Save);
    fileMenu->addAction("Save &As...", this, &MainWindow::onSaveAsFile, QKeySequence::SaveAs);
    fileMenu->addSeparator();

    m_recentMenu = fileMenu->addMenu("Recent Files");
    updateRecentFilesMenu();
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &MainWindow::onExit, QKeySequence::Quit);

    // ══ Edit ══
    QMenu *editMenu = mb->addMenu("&Edit");

    QAction *undoAct = m_undoStack->createUndoAction(this, "&Undo");
    undoAct->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAct);

    QAction *redoAct = m_undoStack->createRedoAction(this, "&Redo");
    redoAct->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();
    editMenu->addAction("Select &All",   this, &MainWindow::onSelectAll,    QKeySequence::SelectAll);
    editMenu->addAction("&Deselect All", this, &MainWindow::onDeselectAll);
    editMenu->addSeparator();
    editMenu->addAction("&Delete Selected", this, &MainWindow::onDeleteSelected, QKeySequence::Delete);

    // ══ View ══
    QMenu *viewMenu = mb->addMenu("&View");

    m_searchPanelAct = viewMenu->addAction("Toggle &Search Panel", this, &MainWindow::onToggleSearchPanel);
    m_searchPanelAct->setCheckable(true);
    m_searchPanelAct->setChecked(true);

    m_statusBarAct = viewMenu->addAction("Toggle Status &Bar", this, &MainWindow::onToggleStatusBar);
    m_statusBarAct->setCheckable(true);
    m_statusBarAct->setChecked(true);

    m_toolbarAct = viewMenu->addAction("Toggle &Toolbar", this, &MainWindow::onToggleToolbar);
    m_toolbarAct->setCheckable(true);
    m_toolbarAct->setChecked(true);

    // ══ Backup ══
    QMenu *backupMenu = mb->addMenu("&Backup");

    backupMenu->addAction("Create Backup &Now", this, &MainWindow::onCreateBackup);
    backupMenu->addAction("&Restore Backup...", this, &MainWindow::onRestoreBackup);
    backupMenu->addSeparator();
    m_autoBackupAct = backupMenu->addAction("&Auto-Backup on Save", this, &MainWindow::onToggleAutoBackup);
    m_autoBackupAct->setCheckable(true);
    m_autoBackupAct->setChecked(true);
    backupMenu->addAction("Open Backup &Folder", this, &MainWindow::onOpenBackupFolder);

    // ══ Tools ══
    QMenu *toolsMenu = mb->addMenu("&Tools");

    toolsMenu->addAction("&Bulk Edit Selected...", this, &MainWindow::onBulkEditSelected, Qt::CTRL | Qt::Key_B);
    toolsMenu->addAction("&Add New Item",          this, &MainWindow::onAddNewItem,       Qt::CTRL | Qt::Key_N);
    toolsMenu->addAction("&Duplicate Selected",    this, &MainWindow::onDuplicateSelected, Qt::CTRL | Qt::Key_D);
    toolsMenu->addSeparator();
    toolsMenu->addAction("Sort by &Name",          this, &MainWindow::onSortByName);
    toolsMenu->addAction("&Merge with XML File...", this, &MainWindow::onMergeXml);

    // ══ Help ══
    QMenu *helpMenu = mb->addMenu("&Help");
    helpMenu->addAction("&About", this, [this]() {
        QDialog dlg(this);
        dlg.setWindowTitle("About DayZ Types Editor");
        dlg.setMinimumWidth(440);

        auto *layout = new QVBoxLayout(&dlg);

        auto *title = new QLabel(
            "<h2>DayZ Types Editor v1.0.0</h2>"
            "<p>A professional editor for DayZ <b>types.xml</b> loot configuration files.</p>"
            "<p>Built with <b>C++17</b> and <b>Qt6</b>.</p>", &dlg);
        title->setWordWrap(true);
        title->setTextFormat(Qt::RichText);
        layout->addWidget(title);

        auto *credit = new QLabel(
            QStringLiteral("Coded by <a href=\"https://github.com/konpep-dev\">\U0001D4F4\U0001D4F8\U0001D4F7\U0001D4F9\U0001D4EE\U0001D4F9\U0001D57\U0001D50</a>"),
            &dlg);
        credit->setTextFormat(Qt::RichText);
        credit->setOpenExternalLinks(true);
        credit->setTextInteractionFlags(Qt::TextBrowserInteraction);
        layout->addWidget(credit);

        auto *license = new QLabel(
            "Licensed under the <b>MIT License</b>.<br>"
            "Free to use, modify, and distribute.", &dlg);
        license->setWordWrap(true);
        license->setTextFormat(Qt::RichText);
        layout->addWidget(license);

        auto *features = new QLabel(
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>Parse and edit all type fields</li>"
            "<li>Bulk editing with set/add/multiply modes</li>"
            "<li>Automatic backups with restore</li>"
            "<li>Full undo/redo support</li>"
            "<li>Search and filter panel</li>"
            "<li>Merge multiple XML files</li>"
            "</ul>", &dlg);
        features->setWordWrap(true);
        features->setTextFormat(Qt::RichText);
        layout->addWidget(features);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
        connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        layout->addWidget(buttonBox);

        dlg.exec();
    });
}

// ────────────────────────────────────────────────────────────
// buildToolbar
// ────────────────────────────────────────────────────────────
void MainWindow::buildToolbar()
{
    m_toolbar = addToolBar("Main Toolbar");
    m_toolbar->setObjectName("MainToolbar");
    m_toolbar->setMovable(false);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_toolbar->setIconSize(QSize(22, 22));

    auto mkAct = [&](const QString &text, const QString &tip) -> QAction* {
        QAction *act = m_toolbar->addAction(text);
        act->setToolTip(tip);
        return act;
    };

    QAction *openAct = mkAct("Open",   "Open types.xml (Ctrl+O)");
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);

    QAction *saveAct = mkAct("Save",   "Save (Ctrl+S)");
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveFile);

    m_toolbar->addSeparator();

    QAction *addAct = mkAct("Add",     "Add New Item (Ctrl+N)");
    connect(addAct, &QAction::triggered, this, &MainWindow::onAddNewItem);

    QAction *delAct = mkAct("Delete",  "Delete Selected (Del)");
    connect(delAct, &QAction::triggered, this, &MainWindow::onDeleteSelected);

    QAction *dupAct = mkAct("Duplicate","Duplicate Selected (Ctrl+D)");
    connect(dupAct, &QAction::triggered, this, &MainWindow::onDuplicateSelected);

    m_toolbar->addSeparator();

    QAction *bulkAct = mkAct("Bulk Edit","Bulk Edit Selected (Ctrl+B)");
    connect(bulkAct, &QAction::triggered, this, &MainWindow::onBulkEditSelected);

    m_toolbar->addSeparator();

    QAction *backupAct = mkAct("Backup",  "Create Backup Now");
    connect(backupAct, &QAction::triggered, this, &MainWindow::onCreateBackup);

    QAction *searchAct = mkAct("Search", "Focus Search Panel (Ctrl+F)");
    connect(searchAct, &QAction::triggered, this, &MainWindow::focusSearch);
}

// ────────────────────────────────────────────────────────────
// buildCentralWidget
// ────────────────────────────────────────────────────────────
void MainWindow::buildCentralWidget()
{
    // Outer vertical splitter: top = (search + table), bottom = detail
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    m_mainSplitter->setObjectName("MainSplitter");

    // ── Top half: horizontal splitter search | table ──
    m_splitter = new QSplitter(Qt::Horizontal, m_mainSplitter);
    m_splitter->setObjectName("TopSplitter");

    // Search panel
    m_searchWidget = new SearchFilterWidget(m_splitter);
    m_searchWidget->setModels(m_model, m_proxyModel);
    m_splitter->addWidget(m_searchWidget);

    // Table view
    m_tableView = new QTableView(m_splitter);
    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setDefaultSectionSize(24);
    m_tableView->verticalHeader()->hide();
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_tableView->sortByColumn(COL_NAME, Qt::AscendingOrder);

    // Column widths
    m_tableView->setColumnWidth(COL_NAME,     180);
    m_tableView->setColumnWidth(COL_NOMINAL,   60);
    m_tableView->setColumnWidth(COL_LIFETIME,  70);
    m_tableView->setColumnWidth(COL_RESTOCK,   60);
    m_tableView->setColumnWidth(COL_MIN,       50);
    m_tableView->setColumnWidth(COL_QUANTMIN,  70);
    m_tableView->setColumnWidth(COL_QUANTMAX,  70);
    m_tableView->setColumnWidth(COL_COST,      55);
    m_tableView->setColumnWidth(COL_CATEGORY, 100);
    m_tableView->setColumnWidth(COL_USAGES,   120);
    m_tableView->setColumnWidth(COL_VALUES,   100);

    m_splitter->addWidget(m_tableView);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({220, 900});

    m_mainSplitter->addWidget(m_splitter);

    // ── Bottom half: detail panel ──
    buildDetailPanel();
    m_mainSplitter->addWidget(m_detailWidget);

    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 0);
    m_mainSplitter->setSizes({550, 280});

    setCentralWidget(m_mainSplitter);

    // ── Connections ──
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(m_tableView, &QTableView::doubleClicked,
            this, &MainWindow::onTableDoubleClicked);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::onTableContextMenu);
    connect(m_model, &TypesModel::itemModified,
            this, &MainWindow::updateStatusBar);
    connect(m_proxyModel, &TypesProxyModel::layoutChanged,
            this, &MainWindow::updateCountLabel);
    connect(m_proxyModel, &TypesProxyModel::rowsInserted,
            this, &MainWindow::updateCountLabel);
    connect(m_proxyModel, &TypesProxyModel::rowsRemoved,
            this, &MainWindow::updateCountLabel);
    connect(m_searchWidget, &SearchFilterWidget::filtersChanged,
            this, &MainWindow::updateCountLabel);
    connect(m_undoStack, &QUndoStack::indexChanged,
            this, [this](int) { updateTitle(); updateStatusBar(); });
}

// ────────────────────────────────────────────────────────────
// buildDetailPanel
// ────────────────────────────────────────────────────────────
void MainWindow::buildDetailPanel()
{
    m_detailWidget = new QWidget(this);
    auto *outerLayout = new QVBoxLayout(m_detailWidget);
    outerLayout->setContentsMargins(4, 4, 4, 4);
    outerLayout->setSpacing(4);

    m_detailGroup = new QGroupBox("Item Details", m_detailWidget);
    outerLayout->addWidget(m_detailGroup);

    auto *scroll = new QScrollArea(m_detailGroup);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *groupLayout = new QVBoxLayout(m_detailGroup);
    groupLayout->setContentsMargins(4, 16, 4, 4);
    groupLayout->addWidget(scroll);

    auto *inner = new QWidget(scroll);
    scroll->setWidget(inner);

    auto *mainLayout = new QHBoxLayout(inner);
    mainLayout->setSpacing(12);

    // ─── Column 1: Basic fields ───
    auto *col1 = new QGroupBox("Basic", inner);
    auto *form1 = new QFormLayout(col1);
    form1->setSpacing(5);

    m_dName     = new QLineEdit(inner);
    m_dNominal  = new QSpinBox(inner);
    m_dLifetime = new QSpinBox(inner);
    m_dRestock  = new QSpinBox(inner);
    m_dMin      = new QSpinBox(inner);
    m_dQuantMin = new QSpinBox(inner);
    m_dQuantMax = new QSpinBox(inner);
    m_dCost     = new QSpinBox(inner);
    m_dCategory = new QLineEdit(inner);
    m_dUsages   = new QLineEdit(inner);
    m_dValues   = new QLineEdit(inner);

    m_dNominal->setRange(-1, 999999);
    m_dLifetime->setRange(-1, 999999);
    m_dRestock->setRange(-1, 999999);
    m_dMin->setRange(-1, 999999);
    m_dQuantMin->setRange(-1, 999999);
    m_dQuantMax->setRange(-1, 999999);
    m_dCost->setRange(-1, 999999);

    m_dUsages->setPlaceholderText("Military, Industrial, ... (comma-separated)");
    m_dValues->setPlaceholderText("Tier1, Tier2, ... (comma-separated)");

    form1->addRow("Name:",     m_dName);
    form1->addRow("Nominal:",  m_dNominal);
    form1->addRow("Lifetime:", m_dLifetime);
    form1->addRow("Restock:",  m_dRestock);
    form1->addRow("Min:",      m_dMin);
    form1->addRow("QuantMin:", m_dQuantMin);
    form1->addRow("QuantMax:", m_dQuantMax);
    form1->addRow("Cost:",     m_dCost);
    form1->addRow("Category:", m_dCategory);
    form1->addRow("Usages:",   m_dUsages);
    form1->addRow("Values:",   m_dValues);

    mainLayout->addWidget(col1, 2);

    // ─── Column 2: Flags ───
    auto *col2 = new QGroupBox("Flags", inner);
    auto *form2 = new QFormLayout(col2);
    form2->setSpacing(5);

    m_dMap     = new QCheckBox("count_in_map",     inner);
    m_dCargo   = new QCheckBox("count_in_cargo",   inner);
    m_dHoarder = new QCheckBox("count_in_hoarder", inner);
    m_dPlayer  = new QCheckBox("count_in_player",  inner);
    m_dCrafted = new QCheckBox("crafted",          inner);
    m_dDeloot  = new QCheckBox("deloot",           inner);

    form2->addRow(m_dMap);
    form2->addRow(m_dCargo);
    form2->addRow(m_dHoarder);
    form2->addRow(m_dPlayer);
    form2->addRow(m_dCrafted);
    form2->addRow(m_dDeloot);

    mainLayout->addWidget(col2, 1);

    // ─── Column 3: Apply/Revert ───
    auto *col3 = new QWidget(inner);
    auto *vl3  = new QVBoxLayout(col3);
    vl3->setAlignment(Qt::AlignTop);
    vl3->setSpacing(8);

    m_dApplyBtn  = new QPushButton("Apply Changes", inner);
    m_dRevertBtn = new QPushButton("Revert",        inner);
    m_dApplyBtn->setMinimumWidth(110);
    m_dRevertBtn->setMinimumWidth(110);

    vl3->addWidget(m_dApplyBtn);
    vl3->addWidget(m_dRevertBtn);
    vl3->addStretch();

    mainLayout->addWidget(col3, 1);

    // ── Connections ──
    connect(m_dApplyBtn,  &QPushButton::clicked, this, &MainWindow::onDetailApply);
    connect(m_dRevertBtn, &QPushButton::clicked, this, &MainWindow::onDetailRevert);

    // Initially disabled until something is selected
    m_detailGroup->setEnabled(false);
}

// ────────────────────────────────────────────────────────────
// buildStatusBar
// ────────────────────────────────────────────────────────────
void MainWindow::buildStatusBar()
{
    m_statusFile   = new QLabel("No file open", this);
    m_statusItems  = new QLabel("0 items", this);
    m_statusBackup = new QLabel("Auto-backup: ON", this);

    statusBar()->addWidget(m_statusFile,   2);
    statusBar()->addWidget(m_statusItems,  2);
    statusBar()->addPermanentWidget(m_statusBackup, 1);
}

// ────────────────────────────────────────────────────────────
// setupShortcuts
// ────────────────────────────────────────────────────────────
void MainWindow::setupShortcuts()
{
    // F5 — reload
    auto *reloadAct = new QAction(this);
    reloadAct->setShortcut(Qt::Key_F5);
    addAction(reloadAct);
    connect(reloadAct, &QAction::triggered, this, [this]() {
        if (!m_currentFilePath.isEmpty()) {
            if (promptSaveIfModified())
                loadFile(m_currentFilePath);
        }
    });

    // Ctrl+F — focus search
    auto *searchFocus = new QAction(this);
    searchFocus->setShortcut(Qt::CTRL | Qt::Key_F);
    addAction(searchFocus);
    connect(searchFocus, &QAction::triggered, this, [this]() {
        if (m_searchWidget) m_searchWidget->focusSearch();
    });
}

// ────────────────────────────────────────────────────────────
// setupAutoBackupTimer — auto-backup every 10 minutes
// ────────────────────────────────────────────────────────────
void MainWindow::setupAutoBackupTimer()
{
    m_autoBackupTimer = new QTimer(this);
    m_autoBackupTimer->setInterval(10 * 60 * 1000); // 10 minutes
    connect(m_autoBackupTimer, &QTimer::timeout, this, &MainWindow::onAutoBackupTimer);
    m_autoBackupTimer->start();
}

// ════════════════════════════════════════════════════════════
//  closeEvent
// ════════════════════════════════════════════════════════════
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!promptSaveIfModified()) {
        event->ignore();
        return;
    }

    // Save settings
    QSettings settings;
    settings.setValue("geometry",    saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("recentFiles", m_recentFiles);
    settings.setValue("autoBackup",  m_backupManager->autoBackupEnabled());

    event->accept();
}

// ════════════════════════════════════════════════════════════
//  File operations
// ════════════════════════════════════════════════════════════
void MainWindow::openFileOnStartup(const QString &path)
{
    if (!path.isEmpty() && QFile::exists(path))
        loadFile(path);
}

void MainWindow::onNewFile()
{
    if (!promptSaveIfModified()) return;

    m_model->setItems({});
    m_currentFilePath.clear();
    m_undoStack->clear();
    m_searchWidget->refreshDropdowns();
    updateTitle();
    updateStatusBar();
    updateCountLabel();
}

void MainWindow::onOpenFile()
{
    if (!promptSaveIfModified()) return;

    const QString path = QFileDialog::getOpenFileName(
        this, "Open types.xml", QString(), "XML Files (*.xml);;All Files (*)");

    if (!path.isEmpty())
        loadFile(path);
}

void MainWindow::loadFile(const QString &path)
{
    QVector<TypeItem> items = m_parser->parseFile(path);

    if (!m_parser->lastError().isEmpty()) {
        QMessageBox::critical(this, "Error loading file", m_parser->lastError());
        return;
    }

    m_model->setItems(items);
    m_currentFilePath = path;
    m_undoStack->clear();
    m_searchWidget->refreshDropdowns();
    m_searchWidget->updateCountLabel(m_proxyModel->rowCount(), m_model->rowCount());

    addRecentFile(path);
    updateTitle();
    updateStatusBar();
    updateCountLabel();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty()) {
        onSaveAsFile();
        return;
    }
    doSave(m_currentFilePath);
}

void MainWindow::onSaveAsFile()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Save As", m_currentFilePath, "XML Files (*.xml);;All Files (*)");

    if (!path.isEmpty())
        doSave(path);
}

void MainWindow::doSave(const QString &path)
{
    // Auto-backup is enabled
    if (m_backupManager->autoBackupEnabled() && QFile::exists(path)) {
        m_backupManager->createBackup(path);
    }

    if (!m_parser->saveFile(path, m_model->items())) {
        QMessageBox::critical(this, "Save Error", m_parser->lastError());
        return;
    }

    m_currentFilePath = path;
    m_model->clearModifiedFlags();
    m_undoStack->setClean();
    addRecentFile(path);
    updateTitle();
    updateStatusBar();
}

void MainWindow::onOpenRecentFile(const QString &path)
{
    if (!promptSaveIfModified()) return;

    if (!QFile::exists(path)) {
        QMessageBox::warning(this, "File Not Found",
            QString("File not found:\n%1").arg(path));
        m_recentFiles.removeAll(path);
        updateRecentFilesMenu();
        return;
    }
    loadFile(path);
}

void MainWindow::onExit()
{
    close();
}

// ════════════════════════════════════════════════════════════
//  Edit operations
// ════════════════════════════════════════════════════════════
void MainWindow::onSelectAll()
{
    m_tableView->selectAll();
}

void MainWindow::onDeselectAll()
{
    m_tableView->clearSelection();
}

void MainWindow::onDeleteSelected()
{
    const QList<int> rows = selectedSourceRows();
    if (rows.isEmpty()) return;

    if (rows.size() > 1) {
        if (QMessageBox::question(this, "Delete",
            QString("Delete %1 item(s)?").arg(rows.size()),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    m_undoStack->push(new RemoveItemsCommand(m_model, rows));
    updateStatusBar();
    updateCountLabel();
}

// ════════════════════════════════════════════════════════════
//  View toggles
// ════════════════════════════════════════════════════════════
void MainWindow::onToggleSearchPanel()
{
    if (m_searchWidget)
        m_searchWidget->setVisible(m_searchPanelAct->isChecked());
}

void MainWindow::onToggleStatusBar()
{
    statusBar()->setVisible(m_statusBarAct->isChecked());
}

void MainWindow::onToggleToolbar()
{
    if (m_toolbar)
        m_toolbar->setVisible(m_toolbarAct->isChecked());
}

// ════════════════════════════════════════════════════════════
//  Backup operations
// ════════════════════════════════════════════════════════════
void MainWindow::onCreateBackup()
{
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::information(this, "Backup", "Please open a file first.");
        return;
    }

    // First save if modified
    if (m_model->modifiedCount() > 0) {
        const auto ans = QMessageBox::question(this, "Backup",
            "There are unsaved changes. Save before backup?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ans == QMessageBox::Cancel) return;
        if (ans == QMessageBox::Yes) doSave(m_currentFilePath);
    }

    const QString backupPath = m_backupManager->createBackup(m_currentFilePath);
    if (backupPath.isEmpty()) {
        QMessageBox::critical(this, "Backup Failed", m_backupManager->lastError());
    } else {
        statusBar()->showMessage(
            QString("Backup created: %1").arg(QFileInfo(backupPath).fileName()), 4000);
    }
}

void MainWindow::onRestoreBackup()
{
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::information(this, "Restore Backup", "Please open a file first.");
        return;
    }

    const QStringList backups = m_backupManager->listBackups(m_currentFilePath);
    if (backups.isEmpty()) {
        QMessageBox::information(this, "Restore Backup", "No backups found.");
        return;
    }

    // Dialog with the backup list
    QDialog dlg(this);
    dlg.setWindowTitle("Restore Backup");
    dlg.setMinimumWidth(420);

    auto *layout = new QVBoxLayout(&dlg);
    layout->addWidget(new QLabel("Select a backup to restore:", &dlg));

    auto *list = new QListWidget(&dlg);
    for (const QString &bp : backups) {
        list->addItem(QFileInfo(bp).fileName());
    }
    if (list->count() > 0) list->setCurrentRow(0);
    layout->addWidget(list);

    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;

    const int idx = list->currentRow();
    if (idx < 0 || idx >= backups.size()) return;

    if (QMessageBox::question(this, "Restore Backup",
        QString("Restore backup:\n%1\n\nThis will overwrite the current file. Continue?")
            .arg(QFileInfo(backups[idx]).fileName()),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    if (!m_backupManager->restoreBackup(backups[idx], m_currentFilePath)) {
        QMessageBox::critical(this, "Restore Failed", m_backupManager->lastError());
    } else {
        loadFile(m_currentFilePath);
        statusBar()->showMessage("Backup restored successfully.", 4000);
    }
}

void MainWindow::onToggleAutoBackup()
{
    const bool enabled = m_autoBackupAct->isChecked();
    m_backupManager->setAutoBackupEnabled(enabled);
    m_statusBackup->setText(enabled ? "Auto-backup: ON" : "Auto-backup: OFF");
}

void MainWindow::onOpenBackupFolder()
{
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::information(this, "Backup Folder", "Please open a file first.");
        return;
    }
    m_backupManager->openBackupFolder(m_currentFilePath);
}

// ════════════════════════════════════════════════════════════
//  Tools operations
// ════════════════════════════════════════════════════════════
void MainWindow::onBulkEditSelected()
{
    const QList<int> rows = selectedSourceRows();
    if (rows.isEmpty()) {
        QMessageBox::information(this, "Bulk Edit", "Please select items to edit.");
        return;
    }

    QVector<TypeItem> selectedItems;
    for (int r : rows) selectedItems.append(m_model->item(r));

    BulkEditDialog dlg(selectedItems, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const auto edits = dlg.allEdits();

    // Check if any field is enabled
    bool anyEnabled = false;
    for (const auto &fe : edits) {
        if (fe.enabled) { anyEnabled = true; break; }
    }
    if (!anyEnabled) return;

    QVector<TypeItem> newItems;
    for (const TypeItem &item : selectedItems) {
        newItems.append(BulkEditDialog::applyEdits(item, edits));
    }

    m_undoStack->push(new BulkEditCommand(m_model, rows, newItems));
    m_searchWidget->refreshDropdowns();
    updateStatusBar();
}

void MainWindow::onAddNewItem()
{
    TypeItem newItem;
    newItem.name     = "NewItem";
    newItem.nominal  = 5;
    newItem.lifetime = 14400;
    newItem.restock  = 1800;
    newItem.min      = 2;
    newItem.quantmin = -1;
    newItem.quantmax = -1;
    newItem.cost     = 100;
    newItem.count_in_map = 1;
    newItem.modified = true;

    const int row = m_model->rowCount();
    m_undoStack->push(new AddItemCommand(m_model, row, newItem));

    // Select the new item
    const QModelIndex proxyIdx = m_proxyModel->mapFromSource(m_model->index(row, 0));
    m_tableView->scrollTo(proxyIdx);
    m_tableView->setCurrentIndex(proxyIdx);
    m_tableView->edit(proxyIdx);

    m_searchWidget->refreshDropdowns();
    updateStatusBar();
    updateCountLabel();
}

void MainWindow::onDuplicateSelected()
{
    const QList<int> rows = selectedSourceRows();
    if (rows.isEmpty()) return;

    const int srcRow = rows.last();
    TypeItem dup = m_model->item(srcRow);
    dup.name    += "_copy";
    dup.modified = true;

    const int insertRow = srcRow + 1;
    m_undoStack->push(new AddItemCommand(m_model, insertRow, dup));

    const QModelIndex proxyIdx = m_proxyModel->mapFromSource(m_model->index(insertRow, 0));
    m_tableView->scrollTo(proxyIdx);
    m_tableView->setCurrentIndex(proxyIdx);

    m_searchWidget->refreshDropdowns();
    updateStatusBar();
    updateCountLabel();
}

void MainWindow::onSortByName()
{
    m_proxyModel->sort(COL_NAME, Qt::AscendingOrder);
    m_tableView->horizontalHeader()->setSortIndicator(COL_NAME, Qt::AscendingOrder);
}

void MainWindow::onMergeXml()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Merge XML File", QString(), "XML Files (*.xml);;All Files (*)");

    if (path.isEmpty()) return;

    XmlParser parser2;
    QVector<TypeItem> mergeItems = parser2.parseFile(path);

    if (!parser2.lastError().isEmpty()) {
        QMessageBox::critical(this, "Merge Error", parser2.lastError());
        return;
    }

    // Merge: items with the same name replace existing ones
    int added = 0, updated = 0;
    for (const TypeItem &mi : mergeItems) {
        bool found = false;
        for (int r = 0; r < m_model->rowCount(); ++r) {
            if (m_model->item(r).name.compare(mi.name, Qt::CaseInsensitive) == 0) {
                TypeItem updated_item = mi;
                updated_item.modified = true;
                m_undoStack->push(new EditItemCommand(m_model, r, m_model->item(r), updated_item, "Merge"));
                ++updated;
                found = true;
                break;
            }
        }
        if (!found) {
            TypeItem ni = mi;
            ni.modified = true;
            m_undoStack->push(new AddItemCommand(m_model, m_model->rowCount(), ni));
            ++added;
        }
    }

    m_searchWidget->refreshDropdowns();
    updateStatusBar();
    updateCountLabel();
    QMessageBox::information(this, "Merge Complete",
        QString("Merge complete.\nAdded: %1 | Updated: %2").arg(added).arg(updated));
}

// ════════════════════════════════════════════════════════════
//  Table interaction
// ════════════════════════════════════════════════════════════
void MainWindow::onSelectionChanged()
{
    const QList<int> rows = selectedSourceRows();

    if (rows.size() == 1) {
        m_detailSourceRow = rows.first();
        loadItemToDetailPanel(m_model->item(m_detailSourceRow));
        m_detailGroup->setEnabled(true);
    } else if (rows.isEmpty()) {
        m_detailSourceRow = -1;
        m_detailGroup->setEnabled(false);
    } else {
        // Multiple selection — disable the detail panel
        m_detailSourceRow = -1;
        m_detailGroup->setEnabled(false);
    }

    updateStatusBar();
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    // Inline editing is handled automatically by QTableView
    // No further action is needed
}

void MainWindow::onTableContextMenu(const QPoint &pos)
{
    const QModelIndex idx = m_tableView->indexAt(pos);
    const QList<int> rows = selectedSourceRows();

    QMenu menu(this);

    if (idx.isValid()) {
        menu.addAction("Edit in Detail Panel", this, [this, &rows]() {
            if (!rows.isEmpty()) {
                m_detailSourceRow = rows.first();
                loadItemToDetailPanel(m_model->item(m_detailSourceRow));
                m_detailGroup->setEnabled(true);
            }
        });
        menu.addAction("Duplicate", this, &MainWindow::onDuplicateSelected);
        menu.addAction("Delete",    this, &MainWindow::onDeleteSelected);
        menu.addSeparator();
    }

    menu.addAction("Bulk Edit Selection...", this, &MainWindow::onBulkEditSelected);
    menu.addAction("Add New Item",           this, &MainWindow::onAddNewItem);
    menu.addSeparator();
    menu.addAction("Select All",   this, &MainWindow::onSelectAll);
    menu.addAction("Deselect All", this, &MainWindow::onDeselectAll);

    menu.exec(m_tableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onProxyDataChanged()
{
    updateCountLabel();
}

// ════════════════════════════════════════════════════════════
//  Detail panel
// ════════════════════════════════════════════════════════════
void MainWindow::loadItemToDetailPanel(const TypeItem &item)
{
    // Block signals to avoid unwanted changes
    m_dName->blockSignals(true);
    m_dName->setText(item.name);
    m_dName->blockSignals(false);

    m_dNominal->setValue(item.nominal);
    m_dLifetime->setValue(item.lifetime);
    m_dRestock->setValue(item.restock);
    m_dMin->setValue(item.min);
    m_dQuantMin->setValue(item.quantmin);
    m_dQuantMax->setValue(item.quantmax);
    m_dCost->setValue(item.cost);

    m_dCargo->setChecked(item.count_in_cargo);
    m_dHoarder->setChecked(item.count_in_hoarder);
    m_dMap->setChecked(item.count_in_map);
    m_dPlayer->setChecked(item.count_in_player);
    m_dCrafted->setChecked(item.crafted);
    m_dDeloot->setChecked(item.deloot);

    m_dCategory->setText(item.category);
    m_dUsages->setText(item.usages.join(", "));
    m_dValues->setText(item.values.join(", "));
}

TypeItem MainWindow::itemFromDetailPanel() const
{
    TypeItem item;
    item.name     = m_dName->text().trimmed();
    item.nominal  = m_dNominal->value();
    item.lifetime = m_dLifetime->value();
    item.restock  = m_dRestock->value();
    item.min      = m_dMin->value();
    item.quantmin = m_dQuantMin->value();
    item.quantmax = m_dQuantMax->value();
    item.cost     = m_dCost->value();

    item.count_in_cargo   = m_dCargo->isChecked()   ? 1 : 0;
    item.count_in_hoarder = m_dHoarder->isChecked() ? 1 : 0;
    item.count_in_map     = m_dMap->isChecked()     ? 1 : 0;
    item.count_in_player  = m_dPlayer->isChecked()  ? 1 : 0;
    item.crafted          = m_dCrafted->isChecked() ? 1 : 0;
    item.deloot           = m_dDeloot->isChecked()  ? 1 : 0;

    item.category = m_dCategory->text().trimmed();

    const QString usagesText = m_dUsages->text().trimmed();
    item.usages = usagesText.isEmpty() ? QStringList{}
                                       : usagesText.split(',', Qt::SkipEmptyParts);
    for (auto &u : item.usages) u = u.trimmed();

    const QString valuesText = m_dValues->text().trimmed();
    item.values = valuesText.isEmpty() ? QStringList{}
                                       : valuesText.split(',', Qt::SkipEmptyParts);
    for (auto &v : item.values) v = v.trimmed();

    item.modified = true;
    return item;
}

void MainWindow::onDetailApply()
{
    if (m_detailSourceRow < 0 || m_detailSourceRow >= m_model->rowCount())
        return;

    const TypeItem oldItem = m_model->item(m_detailSourceRow);
    TypeItem newItem       = itemFromDetailPanel();

    if (oldItem == newItem) return; // No change

    m_undoStack->push(new EditItemCommand(m_model, m_detailSourceRow,
                                          oldItem, newItem, "Edit Item Details"));

    m_searchWidget->refreshDropdowns();
    updateStatusBar();
}

void MainWindow::onDetailRevert()
{
    if (m_detailSourceRow >= 0 && m_detailSourceRow < m_model->rowCount())
        loadItemToDetailPanel(m_model->item(m_detailSourceRow));
}

// ════════════════════════════════════════════════════════════
//  Status updates
// ════════════════════════════════════════════════════════════
void MainWindow::updateStatusBar()
{
    // File label
    QString fileText;
    if (m_currentFilePath.isEmpty()) {
        fileText = "No file open";
    } else {
        fileText = QFileInfo(m_currentFilePath).fileName();
        if (!m_undoStack->isClean() || m_model->modifiedCount() > 0)
            fileText += " *";
    }
    m_statusFile->setText(fileText);

    // Items label
    const int total    = m_model->rowCount();
    const int visible  = m_proxyModel->rowCount();
    const int selected = m_tableView->selectionModel()->selectedRows().size();
    const int modified = m_model->modifiedCount();

    m_statusItems->setText(
        QString("%1 items | %2 visible | %3 selected | %4 modified")
            .arg(total).arg(visible).arg(selected).arg(modified));

    // Backup label
    const bool ab = m_backupManager->autoBackupEnabled();
    m_statusBackup->setText(ab ? "Auto-backup: ON" : "Auto-backup: OFF");
}

void MainWindow::updateTitle()
{
    QString title = "DayZ Types Editor";
    if (!m_currentFilePath.isEmpty()) {
        title += " — " + QFileInfo(m_currentFilePath).fileName();
        if (!m_undoStack->isClean() || m_model->modifiedCount() > 0)
            title += " *";
    }
    setWindowTitle(title);
}

void MainWindow::updateCountLabel()
{
    m_searchWidget->updateCountLabel(m_proxyModel->rowCount(), m_model->rowCount());
    updateStatusBar();
}

// ════════════════════════════════════════════════════════════
//  Auto-backup timer
// ════════════════════════════════════════════════════════════
void MainWindow::onAutoBackupTimer()
{
    if (!m_backupManager->autoBackupEnabled()) return;
    if (m_currentFilePath.isEmpty()) return;
    if (!QFile::exists(m_currentFilePath)) return;

    m_backupManager->createBackup(m_currentFilePath);
}

// ════════════════════════════════════════════════════════════
//  Helpers
// ════════════════════════════════════════════════════════════
bool MainWindow::promptSaveIfModified()
{
    if (m_undoStack->isClean() && m_model->modifiedCount() == 0)
        return true;

    const int ans = QMessageBox::question(this, "Unsaved Changes",
        "There are unsaved changes. Save before continuing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ans == QMessageBox::Save) {
        onSaveFile();
        return true;
    }
    if (ans == QMessageBox::Discard)
        return true;

    return false; // Cancel
}

QList<int> MainWindow::selectedSourceRows() const
{
    const QModelIndexList sel = m_tableView->selectionModel()->selectedRows();
    QList<int> rows;
    for (const QModelIndex &idx : sel) {
        const QModelIndex srcIdx = m_proxyModel->mapToSource(idx);
        if (srcIdx.isValid())
            rows.append(srcIdx.row());
    }
    return rows;
}

void MainWindow::addRecentFile(const QString &path)
{
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > 5)
        m_recentFiles.removeLast();
    updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu()
{
    if (!m_recentMenu) return;

    m_recentMenu->clear();

    if (m_recentFiles.isEmpty()) {
        QAction *empty = m_recentMenu->addAction("(none)");
        empty->setEnabled(false);
        return;
    }

    for (const QString &path : m_recentFiles) {
        QAction *act = m_recentMenu->addAction(QFileInfo(path).fileName());
        act->setToolTip(path);
        connect(act, &QAction::triggered, this, [this, path]() {
            onOpenRecentFile(path);
        });
    }
    m_recentMenu->addSeparator();
    m_recentMenu->addAction("Clear Recent Files", this, [this]() {
        m_recentFiles.clear();
        updateRecentFilesMenu();
    });
}

int MainWindow::currentTotalCount() const
{
    return m_model->rowCount();
}

int MainWindow::currentVisibleCount() const
{
    return m_proxyModel->rowCount();
}

// focusSearch — helper to focus the search panel
void MainWindow::focusSearch()
{
    if (m_searchWidget) {
        m_searchWidget->setVisible(true);
        m_searchWidget->focusSearch();
    }
}
