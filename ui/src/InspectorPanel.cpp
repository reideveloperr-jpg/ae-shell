#include "InspectorPanel.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

namespace ae_shell::ui {

InspectorPanel::InspectorPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(8, 8, 8, 8);
    outer->setSpacing(10);

    auto* title = new QLabel(tr("No layer selected"), this);
    title->setObjectName("inspectorTitle");
    outer->addWidget(title);

    m_form = new QFormLayout;
    m_form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    outer->addLayout(m_form);

    // Phase 0 placeholder: three sample knobs that don't connect to anything.
    {
        auto* spin = new QDoubleSpinBox(this);
        spin->setRange(0.0, 100.0);
        spin->setValue(50.0);
        spin->setSuffix(QStringLiteral(" %"));
        m_form->addRow(tr("Opacity"), spin);
    }
    {
        auto* spin = new QDoubleSpinBox(this);
        spin->setRange(-360.0, 360.0);
        spin->setValue(0.0);
        spin->setSuffix(QStringLiteral("°"));
        m_form->addRow(tr("Rotation"), spin);
    }
    {
        auto* slider = new QSlider(Qt::Horizontal, this);
        slider->setRange(0, 200);
        slider->setValue(100);
        m_form->addRow(tr("Scale"), slider);
    }

    outer->addStretch(1);
}

}  // namespace ae_shell::ui
