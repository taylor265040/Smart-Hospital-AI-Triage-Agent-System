#include "theme.h"
#include <QFont>

namespace Theme {

QString globalStyleSheet()
{
    return QString(R"(
        /* ── Global ── */
        * {
            font-family: %1;
        }

        QWidget {
            background-color: %2;
            color: %3;
        }

        /* ── Scrollbar ── */
        QScrollBar:vertical {
            background: transparent;
            width: 5px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #D6D3D1;
            border-radius: 3px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A8A29E;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar:horizontal {
            background: transparent;
            height: 5px;
        }
        QScrollBar::handle:horizontal {
            background: #D6D3D1;
            border-radius: 3px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #A8A29E;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }

        /* ── QScrollArea ── */
        QScrollArea {
            border: none;
            background: transparent;
        }

        /* ── QLineEdit / QTextEdit / QPlainTextEdit ── */
        QLineEdit, QTextEdit, QPlainTextEdit {
            background: %4;
            border: 1px solid rgba(0,0,0,0.05);
            border-radius: 12px;
            padding: 8px 12px;
            font-size: %5px;
            color: %3;
            selection-background-color: %6;
            selection-color: white;
        }
        QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
            background: white;
            border: 1px solid %6;
        }

        /* ── QPushButton ── */
        QPushButton {
            border-radius: 12px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: %7px;
        }

        /* ── QComboBox ── */
        QComboBox {
            background: %4;
            border: 1px solid rgba(0,0,0,0.05);
            border-radius: 12px;
            padding: 8px 12px;
            font-size: %5px;
            color: %3;
        }
        QComboBox:focus {
            background: white;
            border: 1px solid %6;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox QAbstractItemView {
            background: white;
            border: 1px solid rgba(0,0,0,0.08);
            border-radius: 8px;
            selection-background-color: %8;
            selection-color: %3;
            outline: none;
        }

        /* ── QCheckBox ── */
        QCheckBox {
            spacing: 6px;
            font-size: %7px;
        }

        /* ── QTableWidget ── */
        QTableWidget {
            background: white;
            border: none;
            gridline-color: rgba(0,0,0,0.04);
            font-size: %7px;
        }
        QTableWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid rgba(0,0,0,0.04);
        }
        QTableWidget::item:selected {
            background: %8;
            color: %3;
        }
        QHeaderView::section {
            background: %4;
            border: none;
            border-bottom: 1px solid rgba(0,0,0,0.06);
            padding: 10px 12px;
            font-weight: bold;
            font-size: 10px;
            color: %9;
            text-transform: uppercase;
        }

        /* ── QTabWidget / QTabBar ── */
        QTabWidget::pane {
            border: none;
            background: white;
        }
        QTabBar::tab {
            background: transparent;
            border: none;
            padding: 10px 20px;
            font-size: %7px;
            font-weight: bold;
            color: %9;
        }
        QTabBar::tab:selected {
            color: %6;
            border-bottom: 3px solid %6;
        }

        /* ── QSplitter ── */
        QSplitter::handle {
            background: rgba(0,0,0,0.06);
        }

        /* ── QToolTip ── */
        QToolTip {
            background: %3;
            color: white;
            border-radius: 8px;
            padding: 6px 10px;
            font-size: %7px;
        }

    )")
    .arg(FontFamily)       // %1
    .arg(Surface)          // %2
    .arg(Ink)              // %3
    .arg(SurfaceAlt)       // %4
    .arg(FontMd)           // %5
    .arg(Brand)            // %6
    .arg(FontSm)           // %7
    .arg(BrandLight)       // %8
    .arg(InkMuted);        // %9
}

void applyTheme(QApplication &app)
{
    app.setStyleSheet(globalStyleSheet());
    QFont defaultFont(FontFamily, FontMd);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);
    app.setFont(defaultFont);
}

} // namespace Theme
