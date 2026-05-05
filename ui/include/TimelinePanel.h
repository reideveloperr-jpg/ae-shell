#pragma once

#include <QWidget>

namespace ae_shell::ui {

// Placeholder for the simplified CapCut-style timeline. Phase 3 will wire it
// up to the active AE composition's layer/keyframe data.
class TimelinePanel : public QWidget {
    Q_OBJECT
public:
    explicit TimelinePanel(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

}  // namespace ae_shell::ui
