#include "TimelinePanel.h"

#include <QPaintEvent>
#include <QPainter>

namespace ae_shell::ui {

TimelinePanel::TimelinePanel(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(180);
    setObjectName("timelinePanel");
}

void TimelinePanel::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.fillRect(rect(), QColor("#1a1a1a"));

    const int trackHeight  = 32;
    const int trackSpacing = 6;
    const int leftMargin   = 12;
    const int topMargin    = 12;
    const int rightMargin  = 12;

    // Time ruler.
    p.setPen(QColor("#666"));
    p.drawLine(leftMargin, topMargin, width() - rightMargin, topMargin);
    for (int x = leftMargin; x < width() - rightMargin; x += 60) {
        p.drawLine(x, topMargin - 4, x, topMargin + 4);
    }

    // Three placeholder tracks.
    const QColor clipColors[] = {
        QColor("#3870c0"), QColor("#c08038"), QColor("#5fa860"),
    };
    for (int i = 0; i < 3; ++i) {
        const int y = topMargin + 16 + i * (trackHeight + trackSpacing);
        p.fillRect(leftMargin, y, width() - leftMargin - rightMargin, trackHeight,
                   QColor("#262626"));
        // A single placeholder clip per track.
        const int clipX = leftMargin + 24 + i * 30;
        const int clipW = 220 - i * 24;
        p.setPen(Qt::NoPen);
        p.setBrush(clipColors[i]);
        p.drawRoundedRect(clipX, y + 3, clipW, trackHeight - 6, 4, 4);
    }
}

}  // namespace ae_shell::ui
