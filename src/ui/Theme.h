#ifndef THEME_H
#define THEME_H

#include <QString>

namespace Theme {

inline QString darkStyleSheet() {
    return R"(
        QWidget {
            background-color: #1a1b20;
            color: #e0e6ed;
            font-family: "Segoe UI", "Tahoma", sans-serif;
            font-size: 12px;
        }

        /* Floating Toolbar Container */
        #toolbarContainer {
            background-color: transparent;
            border: none;
        }

        #contentCard {
            background-color: #1e2026;
            border: 1px solid #2d3139;
            border-radius: 8px;
        }

        #topBarCard {
            background-color: #16181d;
            border: 1px solid #2d3139;
            border-radius: 6px;
        }

        #dragHandle {
            background-color: #252830;
            border-top-left-radius: 7px;
            border-bottom-left-radius: 7px;
            color: #7a8292;
            font-weight: bold;
            padding: 4px;
        }

        /* Buttons general */
        QPushButton {
            background-color: #2b2f3a;
            color: #cfd6e0;
            border: 1px solid #3c4250;
            border-radius: 5px;
            padding: 4px 6px;
            font-size: 11px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #383e4d;
            border-color: #00bcd4;
            color: #ffffff;
        }

        QPushButton:pressed {
            background-color: #1f232b;
            border-color: #0097a7;
        }

        QPushButton::menu-indicator {
            image: none;
            width: 0px;
            height: 0px;
            subcontrol-position: right center;
            subcontrol-origin: padding;
        }

        /* Action Buttons */
        QPushButton#profileBtn {
            background-color: #163656;
            color: #90caf9;
            border: 1px solid #234f7c;
            font-weight: bold;
        }

        QPushButton#profileBtn:hover {
            background-color: #1e4b77;
            border-color: #42a5f5;
            color: #ffffff;
        }

        QPushButton#restartBtn {
            background-color: #1b4931;
            color: #a3f7bf;
            border: 1px solid #287a53;
            font-weight: bold;
        }

        QPushButton#restartBtn:hover {
            background-color: #226141;
            border-color: #3ddc84;
            color: #ffffff;
        }

        QPushButton#cleanBtn {
            background-color: #4a2f1b;
            color: #f7c3a3;
            border: 1px solid #7a4e28;
            font-weight: 500;
        }

        QPushButton#cleanBtn:hover {
            background-color: #633f24;
            border-color: #ff9800;
            color: #ffffff;
        }

        QPushButton#stashBtn {
            background-color: #28374d;
            color: #b0d3f8;
            border: 1px solid #3a5477;
        }

        QPushButton#stashBtn:hover {
            background-color: #354a67;
            border-color: #2196f3;
            color: #ffffff;
        }

        /* Profile Buttons */
        QPushButton.profileBtn {
            background-color: #232731;
            color: #9aa2b1;
            border: 1px solid #343a48;
            padding: 3px 6px;
            font-size: 11px;
        }

        QPushButton.profileBtn:hover {
            background-color: #303644;
            color: #ffffff;
            border-color: #556077;
        }

        QPushButton.profileBtn[active="true"] {
            background-color: #0d47a1;
            color: #ffffff;
            border: 1px solid #29b6f6;
            font-weight: bold;
        }

        /* Pin and window buttons */
        QPushButton.iconBtn {
            background-color: #232731;
            border: 1px solid #343a48;
            color: #8c95a6;
            border-radius: 4px;
            padding: 0px;
            font-size: 11px;
            text-align: center;
        }

        QPushButton#pinBtn[pinned="true"] {
            background-color: #e65100;
            color: #ffffff;
            border: 1px solid #ff9800;
        }

        QPushButton.iconBtn:hover {
            background-color: #2e3442;
            border-color: #00bcd4;
            color: #ffffff;
        }

        QPushButton.iconBtn#closeBtn:hover {
            background-color: #d32f2f;
            border-color: #f44336;
            color: #ffffff;
        }

        /* Context Menu */
        QMenu {
            background-color: #1e2026;
            color: #e0e6ed;
            border: 1px solid #383e4d;
            border-radius: 6px;
            padding: 4px;
        }

        QMenu::item {
            padding: 6px 24px 6px 12px;
            border-radius: 4px;
        }

        QMenu::item:selected {
            background-color: #0d47a1;
            color: #ffffff;
        }

        QMenu::separator {
            height: 1px;
            background: #2e3442;
            margin: 4px 6px;
        }

        /* Tooltip */
        QToolTip {
            background-color: #15171c;
            color: #ffffff;
            border: 1px solid #3c4250;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
        }

        /* Dialogs and Inputs */
        QDialog {
            background-color: #1a1b20;
        }

        QLineEdit, QSpinBox, QComboBox {
            background-color: #252831;
            color: #ffffff;
            border: 1px solid #383e4d;
            border-radius: 4px;
            padding: 5px 8px;
        }

        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #00bcd4;
        }

        QTableWidget, QListWidget {
            background-color: #1e2026;
            border: 1px solid #2e3442;
            gridline-color: #2e3442;
            border-radius: 4px;
            selection-background-color: #0d47a1;
            selection-color: #ffffff;
        }

        QHeaderView::section {
            background-color: #252831;
            color: #9aa2b1;
            padding: 5px;
            border: 1px solid #2e3442;
            font-weight: bold;
        }

        QGroupBox {
            border: 1px solid #2d3139;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
            color: #00bcd4;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }

        QCheckBox {
            color: #cfd6e0;
            spacing: 6px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 3px;
            border: 1px solid #3c4250;
            background-color: #252831;
        }

        QCheckBox::indicator:checked {
            background-color: #00bcd4;
            border-color: #00bcd4;
        }
    )";
}

} // namespace Theme

#endif // THEME_H
