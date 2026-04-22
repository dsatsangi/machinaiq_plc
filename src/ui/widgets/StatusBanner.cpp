#include "StatusBanner.h"
#include <QHBoxLayout>

StatusBanner::StatusBanner(QWidget* parent) : QWidget(parent) {
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(16, 8, 16, 8);

    m_label = new QLabel(this);
    m_label->setObjectName("statusBannerLabel");
    lay->addWidget(m_label);

    setMinimumHeight(56);
    setStatus(CycleStatus::Idle);
}

void StatusBanner::setStatus(CycleStatus status) {
    switch (status) {
        case CycleStatus::OK:
            setProperty("statusClass", "ok");
            m_label->setText("✓  OK");
            break;
        case CycleStatus::NOK:
            setProperty("statusClass", "nok");
            m_label->setText("✗  NOK");
            break;
        case CycleStatus::Running:
            setProperty("statusClass", "running");
            m_label->setText("▶  RUNNING");
            break;
        default:
            setProperty("statusClass", "idle");
            m_label->setText("—  IDLE");
            break;
    }
    // Refresh QSS dynamic property
    style()->unpolish(this);
    style()->polish(this);
}
