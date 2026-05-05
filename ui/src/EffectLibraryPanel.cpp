#include "EffectLibraryPanel.h"

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

namespace ae_shell::ui {

namespace {

// Hard-coded sample categories so the layout reads as "real" until Phase 1
// wires up `AEBridge::EnumerateInstalledEffects()`.
struct SampleEffect {
    const char* category;
    const char* name;
};

constexpr SampleEffect kSamples[] = {
    {"Blur & Sharpen",      "Gaussian Blur"},
    {"Blur & Sharpen",      "Camera Lens Blur"},
    {"Color Correction",    "Curves"},
    {"Color Correction",    "Lumetri Color"},
    {"Distort",             "Mesh Warp"},
    {"Distort",             "Wave Warp"},
    {"Generate",            "Lens Flare"},
    {"Generate",            "Fractal"},
    {"Stylize",             "Glow"},
    {"Stylize",             "Find Edges"},
    {"Time",                "Posterize Time"},
    {"Time",                "Time Displacement"},
    {"Third-party",         "Trapcode Particular  (sample)"},
    {"Third-party",         "Sapphire S_Glow      (sample)"},
};

}  // namespace

EffectLibraryPanel::EffectLibraryPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_search = new QLineEdit(this);
    m_search->setPlaceholderText(tr("Search effects..."));
    m_search->setClearButtonEnabled(true);
    layout->addWidget(m_search);

    m_list = new QListWidget(this);
    m_list->setObjectName("effectsList");
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setUniformItemSizes(true);
    layout->addWidget(m_list, /*stretch=*/1);

    QString currentCategory;
    for (const auto& sample : kSamples) {
        if (currentCategory != QLatin1String(sample.category)) {
            currentCategory = QLatin1String(sample.category);
            auto* header = new QListWidgetItem(currentCategory.toUpper(), m_list);
            header->setFlags(Qt::NoItemFlags);
            QFont f = header->font();
            f.setBold(true);
            f.setPointSize(f.pointSize() - 1);
            header->setFont(f);
        }
        new QListWidgetItem(QLatin1String(sample.name), m_list);
    }

    connect(m_search, &QLineEdit::textChanged, this, [this](const QString& q) {
        const QString needle = q.trimmed().toLower();
        for (int i = 0; i < m_list->count(); ++i) {
            auto* it = m_list->item(i);
            const bool isHeader = !(it->flags() & Qt::ItemIsSelectable);
            it->setHidden(!isHeader && !it->text().toLower().contains(needle));
        }
    });
}

}  // namespace ae_shell::ui
