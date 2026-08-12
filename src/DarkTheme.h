#pragma once

#include <QString>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QStyleFactory>

// ────────────────────────────────────────────────────────────
// DarkTheme — VS Code-inspired dark theme
// ────────────────────────────────────────────────────────────
namespace DarkTheme
{

// Color palette
inline constexpr auto BG_DARK    = "#1e1e1e";
inline constexpr auto BG_MEDIUM  = "#252526";
inline constexpr auto BG_LIGHT   = "#2d2d30";
inline constexpr auto ACCENT     = "#007acc";
inline constexpr auto ACCENT_HOV = "#1a8ad4";
inline constexpr auto TEXT       = "#d4d4d4";
inline constexpr auto TEXT_DIM   = "#858585";
inline constexpr auto SELECTION  = "#264f78";
inline constexpr auto BORDER     = "#3f3f46";
inline constexpr auto MODIFIED   = "#2a2a1e";
inline constexpr auto HOVER      = "#2a2d2e";
inline constexpr auto ERROR_RED  = "#f44747";
inline constexpr auto WARN_YEL   = "#ffe080";

// ─── QSS stylesheet ─────────────────────────────────────────
inline QString stylesheet()
{
    return QStringLiteral(
        // ── Global ──
        "* { color: #d4d4d4; }"

        "QWidget {"
        "  background-color: #1e1e1e;"
        "  color: #d4d4d4;"
        "  font-family: 'Segoe UI', Arial, sans-serif;"
        "  font-size: 9pt;"
        "}"

        // ── MainWindow / central ──
        "QMainWindow { background-color: #1e1e1e; }"
        "QMainWindow::separator {"
        "  background: #3f3f46; width: 1px; height: 1px;"
        "}"

        // ── MenuBar ──
        "QMenuBar {"
        "  background-color: #2d2d30;"
        "  border-bottom: 1px solid #3f3f46;"
        "}"
        "QMenuBar::item { padding: 4px 10px; background: transparent; }"
        "QMenuBar::item:selected { background-color: #3f3f46; }"
        "QMenuBar::item:pressed  { background-color: #007acc; }"

        // ── Menu ──
        "QMenu {"
        "  background-color: #252526;"
        "  border: 1px solid #3f3f46;"
        "}"
        "QMenu::item { padding: 6px 24px 6px 12px; }"
        "QMenu::item:selected { background-color: #094771; }"
        "QMenu::separator { height: 1px; background: #3f3f46; margin: 4px 0; }"
        "QMenu::indicator { width: 16px; height: 16px; }"

        // ── Toolbar ──
        "QToolBar {"
        "  background-color: #2d2d30;"
        "  border-bottom: 1px solid #3f3f46;"
        "  spacing: 4px;"
        "  padding: 2px;"
        "}"
        "QToolBar::separator { background: #3f3f46; width: 1px; margin: 4px 2px; }"
        "QToolButton {"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 3px;"
        "  padding: 4px 8px;"
        "  color: #d4d4d4;"
        "}"
        "QToolButton:hover   { background-color: #2a2d2e; border-color: #3f3f46; }"
        "QToolButton:pressed { background-color: #007acc; }"
        "QToolButton:checked { background-color: #094771; border-color: #007acc; }"

        // ── StatusBar ──
        "QStatusBar {"
        "  background-color: #007acc;"
        "  color: #ffffff;"
        "  border-top: none;"
        "}"
        "QStatusBar::item { border: none; }"
        "QStatusBar QLabel { color: #ffffff; padding: 0 6px; }"

        // ── TableView ──
        "QTableView {"
        "  background-color: #1e1e1e;"
        "  alternate-background-color: #252526;"
        "  gridline-color: #3f3f46;"
        "  selection-background-color: #264f78;"
        "  selection-color: #ffffff;"
        "  border: 1px solid #3f3f46;"
        "}"
        "QTableView::item { padding: 2px 6px; }"
        "QTableView::item:hover { background-color: #2a2d2e; }"
        "QTableView::item:selected { background-color: #264f78; color: #ffffff; }"

        // ── Header ──
        "QHeaderView { background-color: #2d2d30; }"
        "QHeaderView::section {"
        "  background-color: #2d2d30;"
        "  color: #d4d4d4;"
        "  padding: 4px 8px;"
        "  border: none;"
        "  border-right: 1px solid #3f3f46;"
        "  border-bottom: 1px solid #3f3f46;"
        "}"
        "QHeaderView::section:hover { background-color: #3f3f46; }"
        "QHeaderView::section:checked { background-color: #094771; }"
        "QHeaderView::down-arrow { image: none; }"
        "QHeaderView::up-arrow   { image: none; }"

        // ── LineEdit ──
        "QLineEdit {"
        "  background-color: #3c3c3c;"
        "  border: 1px solid #3f3f46;"
        "  border-radius: 3px;"
        "  padding: 3px 6px;"
        "  color: #d4d4d4;"
        "  selection-background-color: #264f78;"
        "}"
        "QLineEdit:focus { border-color: #007acc; }"
        "QLineEdit:read-only { background-color: #2d2d30; color: #858585; }"

        // ── SpinBox ──
        "QSpinBox, QDoubleSpinBox {"
        "  background-color: #3c3c3c;"
        "  border: 1px solid #3f3f46;"
        "  border-radius: 3px;"
        "  padding: 2px 4px;"
        "  color: #d4d4d4;"
        "}"
        "QSpinBox:focus, QDoubleSpinBox:focus { border-color: #007acc; }"
        "QSpinBox::up-button, QDoubleSpinBox::up-button,"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  background-color: #3f3f46; border: none; width: 16px;"
        "}"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover,"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "  background-color: #007acc;"
        "}"

        // ── ComboBox ──
        "QComboBox {"
        "  background-color: #3c3c3c;"
        "  border: 1px solid #3f3f46;"
        "  border-radius: 3px;"
        "  padding: 3px 6px;"
        "  color: #d4d4d4;"
        "}"
        "QComboBox:focus { border-color: #007acc; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox QAbstractItemView {"
        "  background-color: #252526;"
        "  border: 1px solid #3f3f46;"
        "  selection-background-color: #094771;"
        "  color: #d4d4d4;"
        "}"

        // ── CheckBox ──
        "QCheckBox { spacing: 6px; }"
        "QCheckBox::indicator {"
        "  width: 14px; height: 14px;"
        "  border: 1px solid #3f3f46;"
        "  border-radius: 2px;"
        "  background-color: #3c3c3c;"
        "}"
        "QCheckBox::indicator:checked   { background-color: #007acc; border-color: #007acc; }"
        "QCheckBox::indicator:hover     { border-color: #007acc; }"

        // ── PushButton ──
        "QPushButton {"
        "  background-color: #0e639c;"
        "  border: none;"
        "  border-radius: 3px;"
        "  padding: 5px 14px;"
        "  color: #ffffff;"
        "  font-weight: 500;"
        "}"
        "QPushButton:hover   { background-color: #1177bb; }"
        "QPushButton:pressed { background-color: #007acc; }"
        "QPushButton:disabled { background-color: #3f3f46; color: #858585; }"
        "QPushButton:flat {"
        "  background-color: transparent;"
        "  border: 1px solid #3f3f46;"
        "  color: #d4d4d4;"
        "}"
        "QPushButton:flat:hover { background-color: #2a2d2e; }"

        // ── GroupBox ──
        "QGroupBox {"
        "  border: 1px solid #3f3f46;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 4px;"
        "  color: #d4d4d4;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  padding: 0 4px;"
        "  left: 8px;"
        "  color: #9cdcfe;"
        "}"

        // ── Splitter ──
        "QSplitter::handle { background-color: #3f3f46; }"
        "QSplitter::handle:horizontal { width: 2px; }"
        "QSplitter::handle:vertical   { height: 2px; }"
        "QSplitter::handle:hover { background-color: #007acc; }"

        // ── ScrollBar ──
        "QScrollBar:vertical {"
        "  background: #1e1e1e; width: 12px; margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #424242; min-height: 20px; border-radius: 6px; margin: 2px;"
        "}"
        "QScrollBar::handle:vertical:hover { background: #5a5a5a; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal {"
        "  background: #1e1e1e; height: 12px; margin: 0;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: #424242; min-width: 20px; border-radius: 6px; margin: 2px;"
        "}"
        "QScrollBar::handle:horizontal:hover { background: #5a5a5a; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"

        // ── TabWidget / TabBar ──
        "QTabWidget::pane {"
        "  border: 1px solid #3f3f46;"
        "  background: #1e1e1e;"
        "}"
        "QTabBar::tab {"
        "  background: #2d2d30;"
        "  color: #d4d4d4;"
        "  padding: 6px 14px;"
        "  border: 1px solid #3f3f46;"
        "  border-bottom: none;"
        "  border-top-left-radius: 3px;"
        "  border-top-right-radius: 3px;"
        "}"
        "QTabBar::tab:selected { background: #1e1e1e; border-bottom: 2px solid #007acc; }"
        "QTabBar::tab:hover:!selected { background: #3f3f46; }"

        // ── Dialog ──
        "QDialog { background-color: #1e1e1e; }"
        "QDialogButtonBox QPushButton { min-width: 70px; }"

        // ── ToolTip ──
        "QToolTip {"
        "  background-color: #252526;"
        "  color: #d4d4d4;"
        "  border: 1px solid #3f3f46;"
        "  padding: 4px 6px;"
        "}"

        // ── Label ──
        "QLabel { background: transparent; }"

        // ── Frame ──
        "QFrame[frameShape='4'], QFrame[frameShape='5'] {"  // HLine / VLine
        "  color: #3f3f46;"
        "}"
    );
}

// ─── QPalette for Fusion style ───────────────────────────────
inline QPalette palette()
{
    QPalette p;

    const QColor bg(0x1e, 0x1e, 0x1e);
    const QColor bgMid(0x25, 0x25, 0x26);
    const QColor bgLight(0x2d, 0x2d, 0x30);
    const QColor text(0xd4, 0xd4, 0xd4);
    const QColor textDim(0x85, 0x85, 0x85);
    const QColor accent(0x00, 0x7a, 0xcc);
    const QColor sel(0x26, 0x4f, 0x78);
    const QColor border(0x3f, 0x3f, 0x46);
    const QColor highlight(0x26, 0x4f, 0x78);
    const QColor highlightedText(0xff, 0xff, 0xff);

    p.setColor(QPalette::Window,          bgMid);
    p.setColor(QPalette::WindowText,      text);
    p.setColor(QPalette::Base,            bg);
    p.setColor(QPalette::AlternateBase,   bgMid);
    p.setColor(QPalette::ToolTipBase,     bgMid);
    p.setColor(QPalette::ToolTipText,     text);
    p.setColor(QPalette::Text,            text);
    p.setColor(QPalette::Button,          bgLight);
    p.setColor(QPalette::ButtonText,      text);
    p.setColor(QPalette::BrightText,      Qt::white);
    p.setColor(QPalette::Link,            accent);
    p.setColor(QPalette::Highlight,       highlight);
    p.setColor(QPalette::HighlightedText, highlightedText);

    p.setColor(QPalette::Disabled, QPalette::Text,       textDim);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, textDim);
    p.setColor(QPalette::Disabled, QPalette::WindowText, textDim);

    return p;
}

// ─── Apply theme ────────────────────────────────────────────
inline void apply(QApplication &app)
{
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setPalette(palette());
    app.setStyleSheet(stylesheet());
}

} // namespace DarkTheme
