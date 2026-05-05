#pragma once

#include <QWidget>

#include <vector>

class QListWidget;
class QLineEdit;
class QLabel;

namespace ae_shell::ui {

struct EffectInfo;

class EffectLibraryPanel : public QWidget {
    Q_OBJECT
public:
    explicit EffectLibraryPanel(QWidget* parent = nullptr);

    // Replaces the panel contents. Pass an empty vector to fall back to a
    // hard-coded sample list (used by the standalone UI build for design
    // iteration without a live AE host).
    void setEffects(const std::vector<EffectInfo>& effects);

private:
    void rebuild(const std::vector<EffectInfo>& effects);
    void applyFilter(const QString& needle);

    QLineEdit*   m_search = nullptr;
    QListWidget* m_list   = nullptr;
    QLabel*      m_count  = nullptr;
};

}  // namespace ae_shell::ui
