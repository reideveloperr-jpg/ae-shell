#pragma once

#include <QMainWindow>

class QToolBar;
class QStatusBar;

namespace ae_shell::ui {

class EffectLibraryPanel;
class TimelinePanel;
class InspectorPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

public slots:
    // Called via QMetaObject::invokeMethod from any thread that calls
    // ae_shell::ui::set_effect_library().  Reads the latest snapshot from
    // UiHost and rebuilds the left panel.
    void refreshEffectLibrary();

private:
    void buildToolbar();
    void buildPanels();
    void buildStatusBar();

    EffectLibraryPanel* m_library  = nullptr;
    TimelinePanel*      m_timeline = nullptr;
    InspectorPanel*     m_inspector = nullptr;
    QToolBar*           m_toolbar  = nullptr;
    QStatusBar*         m_status   = nullptr;
};

}  // namespace ae_shell::ui
