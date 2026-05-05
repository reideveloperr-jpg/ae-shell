#include "MainWindow.h"

#include "EffectLibraryPanel.h"
#include "InspectorPanel.h"
#include "TimelinePanel.h"
#include "UiHost.h"

#include <QAction>
#include <QDockWidget>
#include <QLabel>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace ae_shell::ui {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("AE Shell");
    resize(1400, 900);

    buildToolbar();
    buildPanels();
    buildStatusBar();

    // Pull whatever effect library the plugin has already pushed to us.
    refreshEffectLibrary();
}

void MainWindow::buildToolbar() {
    m_toolbar = addToolBar("Main");
    m_toolbar->setObjectName("mainToolbar");
    m_toolbar->setMovable(false);

    m_toolbar->addAction(tr("Import"));
    m_toolbar->addAction(tr("Templates"));
    m_toolbar->addSeparator();
    m_toolbar->addAction(tr("Export"));
}

void MainWindow::buildPanels() {
    // Centre: a placeholder for the AE composition viewport. In the real
    // plugin this area is intentionally empty — the user keeps the native
    // AE Composition Viewer visible behind/beside our window.
    auto* centre = new QWidget(this);
    auto* layout = new QVBoxLayout(centre);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* hint = new QLabel(
        tr("AE composition viewport lives in After Effects.\n"
           "This area will host live preview thumbnails in a future phase."),
        centre);
    hint->setAlignment(Qt::AlignCenter);
    hint->setObjectName("viewportHint");
    layout->addWidget(hint);

    setCentralWidget(centre);

    // Left dock: effect library.
    m_library = new EffectLibraryPanel(this);
    auto* libDock = new QDockWidget(tr("Effects"), this);
    libDock->setObjectName("effectsDock");
    libDock->setWidget(m_library);
    libDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, libDock);

    // Right dock: parameter inspector.
    m_inspector = new InspectorPanel(this);
    auto* inspDock = new QDockWidget(tr("Inspector"), this);
    inspDock->setObjectName("inspectorDock");
    inspDock->setWidget(m_inspector);
    inspDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, inspDock);

    // Bottom dock: timeline.
    m_timeline = new TimelinePanel(this);
    auto* tlDock = new QDockWidget(tr("Timeline"), this);
    tlDock->setObjectName("timelineDock");
    tlDock->setWidget(m_timeline);
    tlDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, tlDock);
}

void MainWindow::buildStatusBar() {
    m_status = statusBar();
    m_status->showMessage(tr("AE Shell — Phase 1 (effect library)"));
}

void MainWindow::refreshEffectLibrary() {
    if (!m_library) return;
    m_library->setEffects(current_effect_library());

    if (m_status) {
        const std::size_t n = current_effect_library().size();
        if (n == 0) {
            m_status->showMessage(tr("AE Shell — standalone preview (no AE host)"));
        } else {
            m_status->showMessage(tr("AE Shell — %1 installed effects").arg(n));
        }
    }
}

}  // namespace ae_shell::ui
