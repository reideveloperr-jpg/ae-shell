#pragma once

#include <QWidget>

class QFormLayout;

namespace ae_shell::ui {

// Placeholder parameter inspector. Phase 5 will hook this up to the
// AEGP_KeyframeSuite and stream parameter changes both ways.
class InspectorPanel : public QWidget {
    Q_OBJECT
public:
    explicit InspectorPanel(QWidget* parent = nullptr);

private:
    QFormLayout* m_form = nullptr;
};

}  // namespace ae_shell::ui
