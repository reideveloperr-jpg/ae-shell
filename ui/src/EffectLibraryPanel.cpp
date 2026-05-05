#include "EffectLibraryPanel.h"

#include "UiHost.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <map>

namespace ae_shell::ui {

namespace {

// Standalone-mode fallback so the dev UI looks reasonable when there's no
// AE host pushing real data. Removed once the plugin sends the real list.
std::vector<EffectInfo> sampleLibrary() {
    return {
        {"ADBE Gaussian Blur 2",      "Gaussian Blur",          "Blur & Sharpen",   false},
        {"ADBE Camera Lens Blur",     "Camera Lens Blur",       "Blur & Sharpen",   false},
        {"ADBE Curves2",              "Curves",                 "Color Correction", false},
        {"ADBE Lumetri",              "Lumetri Color",          "Color Correction", false},
        {"ADBE MESH WARP",            "Mesh Warp",              "Distort",          false},
        {"ADBE Wave Warp",            "Wave Warp",              "Distort",          false},
        {"ADBE Lens Flare",           "Lens Flare",             "Generate",         false},
        {"ADBE Fractal",              "Fractal",                "Generate",         false},
        {"ADBE Glow",                 "Glow",                   "Stylize",          false},
        {"ADBE Find Edges",           "Find Edges",             "Stylize",          false},
        {"Trapcode Particular",       "Trapcode Particular",    "Trapcode",         true},
        {"Sapphire S_Glow",           "S_Glow",                 "Sapphire Stylize", true},
    };
}

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
    m_list->setUniformItemSizes(false);
    layout->addWidget(m_list, /*stretch=*/1);

    m_count = new QLabel(this);
    m_count->setObjectName("effectsCount");
    layout->addWidget(m_count);

    connect(m_search, &QLineEdit::textChanged, this, [this](const QString& q) {
        applyFilter(q.trimmed().toLower());
    });

    rebuild({});  // start empty; data lands via setEffects()
}

void EffectLibraryPanel::setEffects(const std::vector<EffectInfo>& effects) {
    rebuild(effects);
    if (!m_search->text().isEmpty()) {
        applyFilter(m_search->text().trimmed().toLower());
    }
}

void EffectLibraryPanel::rebuild(const std::vector<EffectInfo>& effects) {
    m_list->clear();

    const std::vector<EffectInfo>& source = !effects.empty()
        ? effects
        : sampleLibrary();

    // Group by category, preserving insertion order of first occurrence so
    // AE's natural ordering wins; within a category sort alphabetically by
    // display name to make scanning easy.
    std::map<QString, std::vector<const EffectInfo*>> grouped;
    QStringList categoryOrder;
    for (const auto& e : source) {
        QString cat = QString::fromStdString(e.category.empty() ? std::string("Uncategorized")
                                                                : e.category);
        if (!grouped.count(cat)) categoryOrder << cat;
        grouped[cat].push_back(&e);
    }

    for (auto& kv : grouped) {
        std::sort(kv.second.begin(), kv.second.end(),
                  [](const EffectInfo* a, const EffectInfo* b) {
                      return a->display_name < b->display_name;
                  });
    }

    int total = 0;
    for (const auto& cat : categoryOrder) {
        auto* header = new QListWidgetItem(cat.toUpper(), m_list);
        header->setFlags(Qt::NoItemFlags);
        QFont f = header->font();
        f.setBold(true);
        f.setPointSize(std::max(1, f.pointSize() - 1));
        header->setFont(f);

        for (const EffectInfo* e : grouped[cat]) {
            QString label = QString::fromStdString(e->display_name);
            if (e->is_third_party) {
                label += QStringLiteral("   \u2022 3rd party");
            }
            auto* item = new QListWidgetItem(label, m_list);
            item->setData(Qt::UserRole,     QString::fromStdString(e->match_name));
            item->setData(Qt::UserRole + 1, QString::fromStdString(e->display_name));
            item->setToolTip(QString::fromStdString(e->match_name));
            ++total;
        }
    }

    if (m_count) {
        if (effects.empty() && !source.empty()) {
            m_count->setText(tr("Sample data — %1 effects (standalone preview)").arg(total));
        } else {
            m_count->setText(tr("%1 installed effects").arg(total));
        }
    }
}

void EffectLibraryPanel::applyFilter(const QString& needle) {
    for (int i = 0; i < m_list->count(); ++i) {
        auto* it       = m_list->item(i);
        const bool hdr = !(it->flags() & Qt::ItemIsSelectable);
        if (needle.isEmpty()) {
            it->setHidden(false);
            continue;
        }
        if (hdr) {
            it->setHidden(false);
            continue;
        }
        const QString name  = it->data(Qt::UserRole + 1).toString().toLower();
        const QString match = it->data(Qt::UserRole).toString().toLower();
        it->setHidden(!name.contains(needle) && !match.contains(needle));
    }
}

}  // namespace ae_shell::ui
