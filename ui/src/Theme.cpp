#include "Theme.h"

#include <QApplication>
#include <QFile>
#include <QPalette>

namespace ae_shell::ui {

void applyTheme(QApplication* app) {
    if (!app) return;

    // Fusion gives us a consistent baseline that the stylesheet can layer on.
    app->setStyle("Fusion");

    QPalette p;
    p.setColor(QPalette::Window,          QColor("#171717"));
    p.setColor(QPalette::WindowText,      QColor("#dcdcdc"));
    p.setColor(QPalette::Base,            QColor("#1f1f1f"));
    p.setColor(QPalette::AlternateBase,   QColor("#242424"));
    p.setColor(QPalette::ToolTipBase,     QColor("#2a2a2a"));
    p.setColor(QPalette::ToolTipText,     QColor("#dcdcdc"));
    p.setColor(QPalette::Text,            QColor("#dcdcdc"));
    p.setColor(QPalette::Button,          QColor("#2a2a2a"));
    p.setColor(QPalette::ButtonText,      QColor("#dcdcdc"));
    p.setColor(QPalette::Highlight,       QColor("#3a78d2"));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Link,            QColor("#5b8fdc"));
    app->setPalette(p);

    // Pull in the .qss layered on top of the palette.
    QFile qss(":/ui/style.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app->setStyleSheet(QString::fromUtf8(qss.readAll()));
    }
}

}  // namespace ae_shell::ui
